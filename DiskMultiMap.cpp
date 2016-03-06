#include "DiskMultiMap.h"
#include "BinaryFile.h"
#include <functional>

// ------------------------------ //
// ----- ITERATOR FUNCTIONS ----- //
// ------------------------------ //

DiskMultiMap::Iterator::Iterator() {
    m_valid = false;
}

DiskMultiMap::Iterator::Iterator(std::string& filename, BinaryFile::Offset offset, std::string key) {
    m_curr = offset;
    m_filename = filename;
    m_key = key;
    m_valid = true;
}

bool DiskMultiMap::Iterator::isValid() const {
    return m_valid;
}

DiskMultiMap::Iterator& DiskMultiMap::Iterator::operator++() {
    
    // Open binary file
    BinaryFile m_file;
    m_file.openExisting(m_filename);
    
    // Set m_curr to be the next value from current association
    m_file.read(m_curr, m_curr + (2 * (MAX_WORD_LENGTH + 1) ) );
    
    if (m_curr == NULL_BUCKET)
        m_valid = false;
    
    // Close file
    m_file.close();
    
    return *this;
}

MultiMapTuple DiskMultiMap::Iterator::operator*() {
    
    // Open binary file
    BinaryFile m_file;
    m_file.openExisting(m_filename);
    
    char value[MAX_WORD_LENGTH + 1];
    char context[MAX_WORD_LENGTH + 1];
    
    // Read the value and context from the current association
    m_file.read(value, MAX_WORD_LENGTH + 1, m_curr);
    m_file.read(context, MAX_WORD_LENGTH + 1, m_curr + (MAX_WORD_LENGTH + 1));
    
    // Fill up the MultiMapTuple to be returned
    MultiMapTuple m;
    m.key = m_key;
    m.value = value;
    m.context = context;
    
    // Close file
    m_file.close();
    
    return m;
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
    
    m_filename = filename;
    m_totalBuckets = numBuckets;
    
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
        b.head = NULL_BUCKET;
        strcpy(b.key, "");
        
        // Write bucket at the end of the file
        m_file.write(b, m_file.fileLength());
    }
    
    return true;
}

// BOR: O(1), Actual: O(1)
bool DiskMultiMap::openExisting(const std::string& filename) {
    if (m_file.isOpen())    // close previously open file
        m_file.close();
    
    if (!m_file.openExisting(filename)) // failed to find file
        return false;

    m_filename = filename;
    
    // Get total buckets count
    Header h;
    m_file.read(h, 0);
    m_totalBuckets = h.totalBuckets;
    return true;
}

// BOR: O(1), Actual: O(1)
void DiskMultiMap::close() {
    if (m_file.isOpen()) {
        m_file.close();
        m_filename.clear();
    }
}

// BOR: O(N/B), Actual: O(1)
// TODO: MODIFY TO ALLOW INSERTING INTO GAPS
bool DiskMultiMap::insert(const std::string& key, const std::string& value, const std::string& context) {
    
    // Input is too long, return false immediately
    if (key.length() > MAX_WORD_LENGTH || value.length() > MAX_WORD_LENGTH || context.length() > MAX_WORD_LENGTH)
        return false;
    
    // Hash the key to find which bucket to insert into
    BinaryFile::Offset bucketOffset = keyHasher(key);
    
    // Read the bucket
    Bucket b;
    m_file.read(b, bucketOffset);
    
    // Create first association to insert
    Association a;
    strcpy(a.value, value.c_str());         // copy value into association
    strcpy(a.context, context.c_str());     // copy context into association
    
    // Bucket is empty so far (adding first bucket)
    if (!b.used) {
        a.next = NULL_BUCKET;                   // no next value
        b.used = true;
        
        strcpy(b.key, key.c_str()); // copy key into the bucket
    
        m_file.write(a, m_file.fileLength());   // write to end of file (for now)
        
        // ------------------------------------- //
        // TODO: Modify this when reusing spaces //
        // ------------------------------------- //
    }
    
    // Bucket already has an association
    else {
        a.next = b.head;  // set new association's next to bucket's current head
        m_file.write(a, m_file.fileLength());
    }
    
    // Set the bucket's head to be newly inserted association
    b.head = m_file.fileLength() - ASSOCIATION_SIZE;
    
    return true;
}

// BOR: O(N/B) or O(K), Actual:
DiskMultiMap::Iterator DiskMultiMap::search(const std::string& key) {
    return DiskMultiMap::Iterator();
}

// BOR: O(N/B) or O(K), Actual:
int DiskMultiMap::erase(const std::string& key, const std::string& value, const std::string& context) {
    return 0;
}

// ----------------------------- //
// ----- PRIVATE FUNCTIONS ----- //
// ----------------------------- //

BinaryFile::Offset DiskMultiMap::keyHasher(const std::string& key) const {
    std::hash<string> hasher;
    size_t hashValue = hasher(key);
    return HEADER_SIZE + (hashValue % m_totalBuckets) * BUCKET_SIZE;
}
