#include "IntelWeb.h"
#include "DiskMultiMap.h"

IntelWeb::IntelWeb() {
    
}

IntelWeb::~IntelWeb() {
    m_map.close();
}

bool IntelWeb::createNew(const std::string& filePrefix, unsigned int maxDataItems) {
    
}

bool IntelWeb::openExisting(const std::string& filePrefix) {
    
}

void IntelWeb::close() {
    
}

bool IntelWeb::ingest(const std::string& telemetryFile) {
    
}

unsigned int IntelWeb::crawl(const std::vector<std::string>& indicators,
                   unsigned int minPrevalenceToBeGood,
                   std::vector<std::string>& badEntitiesFound,
                   std::vector<InteractionTuple>& badInteractions
                             ) {
    
}

bool IntelWeb::purge(const std::string& entity) {
    
}
