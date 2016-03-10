#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <queue>
#include <string>

#include "IntelWeb.h"
#include "DiskMultiMap.h"

using namespace std;

IntelWeb::IntelWeb() {
    
}

IntelWeb::~IntelWeb() {
    m_toFromMap.close();
    m_fromToMap.close();
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

// BOR: O(lines in file), Actual: O(lines in file)
bool IntelWeb::ingest(const string& telemetryFile) {
    
    ifstream inf(telemetryFile);
    
    if (!inf) {
        cerr << "Cannot open telemetry file!" << endl;
        return false;
    }
    
    string line;
    while (getline(inf, line)) {
        istringstream iss(line);
        string machine, to, from;
        
        // Get the machine, to, and from strings from each line
        if (! (iss >> machine >> to >> from)) {
            cerr << "Ignoring badly-formatted input line: " << line << endl;
            continue;
        }
        
        // Insert entities into two maps:
        //  • to -> from/machine
        //  • from -> to/machine
        m_toFromMap.insert(to, from, machine);
        m_fromToMap.insert(from, to, machine);
        
        // Log prevalence of each entity
        incrementPrevalence(to);
        incrementPrevalence(from);
    }
    
    return true;
}

// Assuming T telemetry lines
// BOR: O(T) disk accesses & O(TlogT) operations involving in-memory data structures
// Actual:
unsigned int IntelWeb::crawl(const vector<string>& indicators, unsigned int minPrevalenceToBeGood, vector<string>& badEntitiesFound, vector<InteractionTuple>& badInteractions) {
    
    set<string> badEntitySet;
    set<InteractionTuple> badInteractionSet;
    queue<string> maliciousQueue;
    
    // Load queue with malicious indicators
    for (int i = 0; i < indicators.size(); ++i) {
        maliciousQueue.push(indicators[i]);
        if (prevalenceOf(indicators[i]) > 0)
            badEntitySet.insert(indicators[i]);
    }       // O(1)
    
    // While there are still malicious entities to check
    while (!maliciousQueue.empty()) {                                   // Runs T times
        string entity = maliciousQueue.front();
        maliciousQueue.pop();
        
        // Look for the entity in the to-from map
        DiskMultiMap::Iterator it = m_toFromMap.search(entity);
        while (it.isValid()) {                                          // Runs T/B times
            MultiMapTuple m = *it;
            if(prevalenceOf(m.value) < minPrevalenceToBeGood)
                if(badEntitySet.insert(m.value).second)         //
                    maliciousQueue.push(m.value);               // need to check against this entity later
            
            // The interaction is bad, regardless of the value (since key was malicious)
            InteractionTuple t;
            t.to = m.value;
            t.from = m.key;
            t.context = m.context;
            badInteractionSet.insert(t);
            ++it;
        }
        
        // Look for the entity in the from-to map
        it = m_fromToMap.search(entity);
        while (it.isValid()) {
            MultiMapTuple m = *it;
            if(prevalenceOf(m.value) < minPrevalenceToBeGood)
                if(badEntitySet.insert(m.value).second)         // returns true if insert was unique
                    maliciousQueue.push(m.value);               // need to check against this entity later
            
            // The interaction is bad, regardless of the value (since key was malicious)
            InteractionTuple t;
            t.to = m.key;
            t.from = m.value;
            t.context = m.context;
            badInteractionSet.insert(t);
            
            ++it;
        }
    }
    
    // Empty the bad entity and interaction vectors
    badEntitiesFound.clear();
    badInteractions.clear();
    
    for (set<string>::iterator itbE = badEntitySet.begin(); itbE != badEntitySet.end(); itbE++)
        badEntitiesFound.push_back(*itbE);
    
    for (set<InteractionTuple>::iterator itBIS = badInteractionSet.begin(); itBIS != badInteractionSet.end(); itBIS++)
        badInteractions.push_back(*itBIS);
    
    return int(badEntitySet.size() + indicators.size());
}

// BOR: O(M), Actual: O(M)
// M is the number of items matching the string parameter
bool IntelWeb::purge(const string& entity) {

    DiskMultiMap::Iterator it = m_fromToMap.search(entity);
    
    // For every association in from-to with entity as the key
    while (it.isValid()) {
        MultiMapTuple m = *it;
        m_fromToMap.erase(m.key, m.value, m.context);   // erase from map1
        m_toFromMap.erase(m.value, m.key, m.context);   // erase from map2
        decrementPrevalence(m.value);
        
        it = m_fromToMap.search(entity);
    }
    
    it = m_toFromMap.search(entity);
    
    // For every association with entity as the key
    while (it.isValid()) {
        MultiMapTuple m = *it;
        m_toFromMap.erase(m.key, m.value, m.context);   // erase from map2
        m_fromToMap.erase(m.value, m.key, m.context);   // erase from map1
        decrementPrevalence(m.value);
        
        it = m_toFromMap.search(entity);
    }
    
    setPrevalenceZero(entity);
    return true;
}

// Private Functions

int IntelWeb::prevalenceOf(string entity) {
    
    DiskMultiMap::Iterator it = m_prevalenceMap.search(entity);
    
    // Prevalence doesn't exist yet
    if (!it.isValid()) {
        return 0;
    } else {
        // Prevalence does exist
        string countString = (*it).value;
        return stoi(countString);
    }
}

void IntelWeb::incrementPrevalence(string entity) {
    int count = prevalenceOf(entity);
    m_prevalenceMap.erase(entity, to_string(count), "");
    m_prevalenceMap.insert(entity, to_string(count + 1), "");
}

void IntelWeb::decrementPrevalence(string entity) {
    int count = prevalenceOf(entity);
    m_prevalenceMap.erase(entity, to_string(count), "");
    m_prevalenceMap.insert(entity, to_string(count - 1), "");
}

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