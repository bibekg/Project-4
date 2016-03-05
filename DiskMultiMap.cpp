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







