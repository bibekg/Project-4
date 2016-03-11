#ifndef INTELWEB_H_
#define INTELWEB_H_

#include "InteractionTuple.h"
#include "DiskMultiMap.h"
#include <string>
#include <vector>
#include <unordered_map>

class IntelWeb
{
public:
    IntelWeb();
    ~IntelWeb();
    bool createNew(const std::string& filePrefix, unsigned int maxDataItems);
    bool openExisting(const std::string& filePrefix);
    void close();
    bool ingest(const std::string& telemetryFile);
    unsigned int crawl(const std::vector<std::string>& indicators,
                       unsigned int minPrevalenceToBeGood,
                       std::vector<std::string>& badEntitiesFound,
                       std::vector<InteractionTuple>& badInteractions
                       );
    bool purge(const std::string& entity);
    
private:
    
    const double LOAD_FACTOR = 0.5;
    const int MAX_CACHE_SIZE = 10000000;
    
    DiskMultiMap m_toFromMap;
    DiskMultiMap m_fromToMap;
    DiskMultiMap m_prevalenceMap;
    
    unordered_map<std::string, int> m_prevCache;

    int getCachedPrevalence(string entity);
    int prevalenceOf(std::string entity);
    void incrementPrevalence(std::string entity);
    void decrementPrevalence(std::string entity);
    void setPrevalenceZero(std::string entity);
};
#endif // INTELWEB_H_