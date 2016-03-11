#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <unordered_set>
#include <queue>
#include <string>
#include <cstring>

#include "IntelWeb.h"
#include "DiskMultiMap.h"

using namespace std;

IntelWeb::IntelWeb() {}

IntelWeb::~IntelWeb() {
    m_toFromMap.close();
    m_fromToMap.close();
    m_prevalenceMap.close();
}

// BOR: O(maxDataItems), Actual: O(maxDataItems)
bool IntelWeb::createNew(const string& filePrefix, unsigned int maxDataItems) {
    
    close();
    
    bool success = true;
    if(!m_toFromMap.createNew(filePrefix + "-to-from.dat", int(maxDataItems/LOAD_FACTOR)))
        success = false;
    
    if (!m_fromToMap.createNew(filePrefix + "-from-to.dat", int(maxDataItems/LOAD_FACTOR)))
        success = false;
    
    if (!m_prevalenceMap.createNew(filePrefix + "-prevalence.dat", int(maxDataItems/LOAD_FACTOR)))
        success = false;
    
    if (!success) {
        close();
        cerr << "Failed to create an IntelWeb!" << endl;
        return false;
    }
    
    return true;
}

// BOR: O(1), Actual: O(1)
bool IntelWeb::openExisting(const string& filePrefix) {
    
    close();
    
    bool success = true;
    
    if (!m_toFromMap.openExisting(filePrefix + "-to-from.dat"))
        success = false;
    if (!m_fromToMap.openExisting(filePrefix + "-from-to.dat"))
        success = false;
    if (!m_prevalenceMap.openExisting(filePrefix + "-prevalence.dat"))
        success = false;
    
    if (!success) {
        close();
        cerr << "Failed to open an existing IntelWeb!" << endl;
        return false;
    }
    
    return true;
}

// BOR: O(1), Actual: O(1)
void IntelWeb::close() {
    m_toFromMap.close();
    m_fromToMap.close();
    m_prevalenceMap.close();
}

// Assuming N is the total number of telemetry data lines
// BOR: O(N), Actual: O(N)
bool IntelWeb::ingest(const string& telemetryFile) {
    
    ifstream inf(telemetryFile);
    
    if (!inf) {
        cerr << "Cannot open telemetry file!" << endl;
        return false;
    }
    
    string line;
    while (getline(inf, line)) {                    // O(N)
        istringstream iss(line);
        string machine, to, from;
        
        if (! (iss >> machine >> to >> from)) {
            cerr << "Ignoring badly-formatted input line: " << line << endl;
            continue;
        }

        m_toFromMap.insert(to, from, machine);
        m_fromToMap.insert(from, to, machine);
        
        // Log prevalence of each entity            // O(1)
        incrementPrevalence(to);
        incrementPrevalence(from);
    }
    
    return true;
}

// Assuming T telemetry lines that refer to malicious entities
// BOR: O(T) disk accesses & O(TlogT) operations involving in-memory data structures
// Actual: O(T) disk accesses, O(TlogT) 
unsigned int IntelWeb::crawl(const vector<string>& indicators, unsigned int minPrevalenceToBeGood, vector<string>& badEntitiesFound, vector<InteractionTuple>& badInteractions) {
    
    unordered_set<string> badEntitySet;
    set<InteractionTuple> badInteractionSet;
    queue<string> maliciousQueue;
    
    // Load queue with malicious indicators                                 // Runs in O(1) time
    for (int i = 0; i < indicators.size(); ++i) {
        maliciousQueue.push(indicators[i]);
        
        // Insert indicators into badEntitySet only
        // if they are present in the telemetry data
        if (prevalenceOf(indicators[i]) > 0)
            badEntitySet.insert(indicators[i]);
    }
    
    // While there are still malicious entities to check                    // Runs in O(T) time
    while (!maliciousQueue.empty()) {
        string entity = maliciousQueue.front();
        maliciousQueue.pop();
        
        // Look for the entity in the to-from map
        DiskMultiMap::Iterator it = m_toFromMap.search(entity);
        while (it.isValid()) {                                      // For every association with that entity as key
            MultiMapTuple m = *it;
            if(getCachedPrevalence(m.value) < minPrevalenceToBeGood && badEntitySet.insert(m.value).second)
                maliciousQueue.push(m.value);                   // need to check against this entity later
            badInteractionSet.insert(InteractionTuple(m.key, m.value, m.context));
            ++it;
        }
        
        // Look for the entity in the from-to map
        it = m_fromToMap.search(entity);
        while (it.isValid()) {                                      // For every association with that entity as key
            MultiMapTuple m = *it;
            if(getCachedPrevalence(m.value) < minPrevalenceToBeGood && badEntitySet.insert(m.value).second)
                maliciousQueue.push(m.value);                   // need to check against this entity later
            badInteractionSet.insert(InteractionTuple(m.value, m.key, m.context));
            ++it;
        }
    }
    
    // Empty the bad entity and interaction vectors
    badEntitiesFound.clear();
    badInteractions.clear();
    
    // Copy bad entities over to vector
    unordered_set<string>::iterator itBES = badEntitySet.begin();           // Runs in O(T)
    while (itBES != badEntitySet.end()) {
        badEntitiesFound.push_back((*itBES));
        ++itBES;
    }
    
    // Copy bad interactions over to vector
    set<InteractionTuple>::iterator itBIS = badInteractionSet.begin();      // Runs in O(T)
    while (itBIS != badInteractionSet.end()) {
        badInteractions.push_back(*itBIS);
        ++itBIS;
    }

    std::sort(badEntitiesFound.begin(), badEntitiesFound.end());                 // Runs in O(TlogT)
    std::sort(badInteractions.begin(), badInteractions.end());                   // Runs in O(TlogT)
    
    return int(badEntitiesFound.size());
}

// Assuming M items matching the string parameter
// BOR: O(M), Actual: O(M)
bool IntelWeb::purge(const string& entity) {
    bool didPurge = false;
    
    // For every association in the from-to map with entity as the key
    DiskMultiMap::Iterator it = m_fromToMap.search(entity);
    while (it.isValid()) {
        didPurge = true;
        MultiMapTuple m = *it;
        m_fromToMap.erase(m.key, m.value, m.context);   // erase from map1
        m_toFromMap.erase(m.value, m.key, m.context);   // erase from map2
        decrementPrevalence(m.value);
        
        it = m_fromToMap.search(entity);
    }
    
    // For every association in the to-from mapwith entity as the key
    it = m_toFromMap.search(entity);
    while (it.isValid()) {
        didPurge = true;
        MultiMapTuple m = *it;
        m_toFromMap.erase(m.key, m.value, m.context);   // erase from map2
        m_fromToMap.erase(m.value, m.key, m.context);   // erase from map1
        decrementPrevalence(m.value);
        
        it = m_toFromMap.search(entity);
    }
    
    setPrevalenceZero(entity);
    
    // Clear cache upon purge since cached values are no longer correct
    if (didPurge) m_prevCache.clear();
    return didPurge;
}

// ----------------------------- //
// ----- PRIVATE FUNCTIONS ----- //
// ----------------------------- //

// Big-O: O(1)
int IntelWeb::getCachedPrevalence(string entity) {
    
    // Clear cache if it has grown too large
    if (m_prevCache.size() > MAX_CACHE_SIZE)
        m_prevCache.clear();
    
    // If the prevalence is cached, return the cached prevalence
    unordered_map<string, int>::iterator itPC = m_prevCache.find(entity);
    if (itPC != m_prevCache.end())
        return (*itPC).second;
    
    DiskMultiMap::Iterator it = m_prevalenceMap.search(entity);
    
    // Prevalence doesn't exist, consider it to be 0
    if (!it.isValid()) {
        
        // Cache prevalence of 0
        std::pair<string, int> p (entity, 0);
        m_prevCache.insert(p);
        
        return 0;
    } else {
        // Prevalence does exist
        string countString = (*it).value;
        int count = stoi(countString);
        
        std::pair<string, int> p (entity, count);   // Cache the prevalence
        m_prevCache.insert(p);
        
        return count;
    }
    
}

// Big-O: O(1)
int IntelWeb::prevalenceOf(string entity) {
    
    DiskMultiMap::Iterator it = m_prevalenceMap.search(entity);
    
    // Prevalence doesn't exist, consider it to be 0
    if (!it.isValid())
        return 0;
    else
        return stoi((*it).value);
}

// Big-O: O(1)
void IntelWeb::incrementPrevalence(string entity) {
    int count = prevalenceOf(entity);
    m_prevalenceMap.erase(entity, to_string(count), "");
    m_prevalenceMap.insert(entity, to_string(count + 1), "");
}

// Big-O: O(1)
void IntelWeb::decrementPrevalence(string entity) {
    int count = prevalenceOf(entity);
    m_prevalenceMap.erase(entity, to_string(count), "");
    m_prevalenceMap.insert(entity, to_string(count - 1), "");
}

// Big-O: O(1)
void IntelWeb::setPrevalenceZero(string entity) {
    int count = prevalenceOf(entity);
    m_prevalenceMap.erase(entity, to_string(count), "");
    m_prevalenceMap.insert(entity, to_string(0), "");
}

bool operator<(const InteractionTuple& t1, const InteractionTuple& t2) {
    
    // Sort by context, then from, then to
    
    if (t1.context < t2.context) {
        return true;
    } else if (t1.context == t2.context) {
        if (t1.from < t2.from) {
            return true;
        } else if (t1.from == t2.from) {
            if (t1.to < t2.to) {
                return true;
            }
        }
    }
    
    return false;
    
}
