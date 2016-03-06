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

// BOR: O(B), Actual: O(B)
bool DiskMultiMap::createNew(const std::string& filename, unsigned int numBuckets) {
    if (m_file.isOpen())
        m_file.close();
    
    if(!m_file.createNew(filename))
        return false;   // return false if failed
    
    // Create hash table header
    DiskMultiMap::Header h;
    h.totalBuckets = numBuckets;
    h.bucketsUsed = 0;
    m_file.write(h, 0);
    
    // Create #numBuckets buckets
    // Runs in O(numBuckets) time
    for (int i = 0; i < numBuckets; i++) {
        Bucket b;
        b.used = false;
        b.first = -1;
        strcpy(b.key, "");
        
        // Write bucket at the end of the file
        m_file.write(b, m_file.fileLength());
    }
    
    return true;
}

// BOR: O(1), Actual: O(1)
bool DiskMultiMap::openExisting(const std::string& filename) {
    if (m_file.isOpen())
        m_file.close();
    
    if (!m_file.openExisting(filename)) // failed to find file
        return false;

    m_filename = filename;
    return true;
}

void DiskMultiMap::close() {
    if (m_file.isOpen()) {
        m_file.close();
        m_filename.clear();
    }
}

bool DiskMultiMap::insert(const std::string& key, const std::string& value, const std::string& context) {
    
    // Input is too long, return false immediately
    if (key.length() > 120 || value.length() > 120 || context.length() > 120)
        return false;
    
    
    
    
    
    return true;
}

DiskMultiMap::Iterator DiskMultiMap::search(const std::string& key) {
    return DiskMultiMap::Iterator();
}

int DiskMultiMap::erase(const std::string& key, const std::string& value, const std::string& context) {
    return 0;
}







