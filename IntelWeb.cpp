#include <iostream> // needed for any I/O
#include <fstream>  // needed in addition to <iostream> for file I/O
#include <sstream>  // needed in addition to <iostream> for string stream I/O

#include "IntelWeb.h"
#include "DiskMultiMap.h"

using namespace std;

IntelWeb::IntelWeb() {
    
}

IntelWeb::~IntelWeb() {
    m_toFromMap.close();
    m_fromToMap.close();
}

bool IntelWeb::createNew(const std::string& filePrefix, unsigned int maxDataItems) {
    
    bool success = true;
    if(!m_toFromMap.createNew(filePrefix + "-to-from.dat", int(maxDataItems/LOAD_FACTOR)))
        success = false;
    
    if (!m_fromToMap.createNew(filePrefix + "-from-to.dat", int(maxDataItems/LOAD_FACTOR)))
        success = false;
    
    if (!success) {
        close();
        return false;
    }
    
    return true;
}

bool IntelWeb::openExisting(const std::string& filePrefix) {
    
    close();
    
    bool success = true;
    
    if (!m_toFromMap.openExisting(filePrefix + "-to-from.dat"))
        success = false;
    if (!m_fromToMap.openExisting(filePrefix + "-from-to.dat"))
        success = false;
    
    if (!success) {
        close();
        return false;
    }
    
    return true;
}

void IntelWeb::close() {
    m_toFromMap.close();
    m_fromToMap.close();
}

bool IntelWeb::ingest(const std::string& telemetryFile) {
    
    ifstream inf(telemetryFile);
    
    if (!inf) {
        cerr << "Cannot open telemetry file!" << endl;
        exit(1);
    }
    
    string line;
    while (getline(inf, line)) {
        istringstream iss(line);
        string machine, to, from;
        
        if (! (iss >> machine >> to >> from)) {
            cerr << "Ignoring badly-formatted input line: " << line << endl;
            continue;
        }
        
        m_toFromMap.insert(to, from, machine);
        m_fromToMap.insert(from, to, machine);
    }
    
    return true;
}

unsigned int IntelWeb::crawl(const std::vector<std::string>& indicators,
                   unsigned int minPrevalenceToBeGood,
                   std::vector<std::string>& badEntitiesFound,
                   std::vector<InteractionTuple>& badInteractions
                             ) {
    return 0;
}

bool IntelWeb::purge(const std::string& entity) {
    return true;
}
