#include "DiskMultiMap.h"
#include "BinaryFile.h"

// ------------------------------ //
// ----- ITERATOR FUNCTIONS ----- //
// ------------------------------ //

DiskMultiMap::Iterator::Iterator() {
    m_valid = false;
}

bool DiskMultiMap::Iterator::isValid() const {
    return m_valid;
}

DiskMultiMap::Iterator& DiskMultiMap::Iterator::operator++() {
    DiskMultiMap::Iterator it;
    return it;
}

MultiMapTuple DiskMultiMap::Iterator::operator*() {
    return MultiMapTuple();
}

// ---------------------------------- //
// ----- DISKMULTIMAP FUNCTIONS ----- //
// ---------------------------------- //

DiskMultiMap::DiskMultiMap() {
    m_filename.clear();
}

DiskMultiMap::~DiskMultiMap() {
    m_file.close();
}

bool DiskMultiMap::createNew(const std::string& filename, unsigned int numBuckets) {
    m_file.close();
    if(!m_file.createNew(filename))
        return false;   // return false if failed
    
    // Create hash table header
    DiskMultiMap::Header h;
    h.totalBuckets = numBuckets;
    h.bucketsUsed = 0;
    m_file.write(h, 0);
    
    // Create B buckets
    for (int i = 0; i < numBuckets; i++) {
        Bucket b;
        b.used = false;
        
        // Write bucket at the end of the file
        m_file.write(b, m_file.fileLength());
    }
    
    return true;
}

bool DiskMultiMap::openExisting(const std::string& filename) {
    return true;
}

void DiskMultiMap::close() {
    
}

bool DiskMultiMap::insert(const std::string& key, const std::string& value, const std::string& context) {
    return true;
}

DiskMultiMap::Iterator DiskMultiMap::search(const std::string& key) {
    return DiskMultiMap::Iterator();
}

int DiskMultiMap::erase(const std::string& key, const std::string& value, const std::string& context) {
    return 0;
}







