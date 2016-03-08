#include "DiskMultiMap.h"
#include "BinaryFile.h"
#include <functional>

// ------------------------------ //
// ----- ITERATOR FUNCTIONS ----- //
// ------------------------------ //

DiskMultiMap::Iterator::Iterator() {
    m_valid = false;
}

DiskMultiMap::Iterator::Iterator(std::string& filename, BinaryFile::Offset offset, std::string key, int gapSize) {
    m_valid = true;
    m_curr = offset;
    m_filename = filename;
    m_key = key;
    m_gapSize = gapSize;
}

bool DiskMultiMap::Iterator::isValid() const {
    return m_valid;
}

DiskMultiMap::Iterator& DiskMultiMap::Iterator::operator++() {
    
    // Open binary file
    BinaryFile m_file;
    m_file.openExisting(m_filename);
    
    // Set m_curr to be the next value from current association
//    BinaryFile::Offset nextOffset = m_curr + 2 * sizeof(char[MAX_WORD_LENGTH + 1]);
    BinaryFile::Offset nextOffset = m_curr + m_gapSize;
    m_file.read(m_curr, nextOffset);
    
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
    close();
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
    if (!m_file.write(h, 0)) cerr << "Failed to write to disk!" << endl;
    
//    {
//        Header h;
//        h.bucketsUsed = 1234;
//        m_file.read(h, 0);
//    }
    
    // Create #numBuckets buckets
    // Runs in O(numBuckets) time
    for (int i = 0; i < numBuckets; i++) {
        Bucket b;
        b.used = false;
        b.head = NULL_BUCKET;
        strcpy(b.key, "");
        
        // Write bucket at the end of the file
        if (!m_file.write(b, m_file.fileLength())) cerr << "Failed to write to disk!" << endl;
    }
    
    close();
    
    return true;
}

// BOR: O(1), Actual: O(1)
bool DiskMultiMap::openExisting(const std::string& filename) {
    if (m_file.isOpen())    // close previously open file
        m_file.close();
    
    if (!m_file.openExisting(filename)) { // failed to find file
     
        cerr << "FAILED TO OPEN FILE FROM DISK!" << endl;
        return false;
        
    }

    m_filename = filename;
    
    // Get total buckets count
    Header h;
    if (!m_file.read(h, 0)) cerr << "Failed to read from disk!";
    m_totalBuckets = h.totalBuckets;
    return true;
}

// BOR: O(1), Actual: O(1)
void DiskMultiMap::close() {
    if (m_file.isOpen()) {
        m_file.close();
    }
}

// BOR: O(N/B), Actual: O(1)
// TODO: MODIFY TO ALLOW INSERTING INTO GAPS
bool DiskMultiMap::insert(const std::string& key, const std::string& value, const std::string& context) {
    
    // Input is too long, return false immediately
    if (key.length() > MAX_WORD_LENGTH || value.length() > MAX_WORD_LENGTH || context.length() > MAX_WORD_LENGTH)
        return false;
    
    openExisting(m_filename);
    
    // Hash the key to find which bucket to insert into
    BinaryFile::Offset bucketOffset = keyHasher(key);
    
    // Read the bucket
    Bucket b;
    if (!m_file.read(b, bucketOffset)) cerr << "Failed to read from disk!" << endl;
    
    // Create first association to insert
    Association a;
    strcpy(a.value, value.c_str());         // copy value into association
    strcpy(a.context, context.c_str());     // copy context into association
    
    BinaryFile::Offset insertHere = m_file.fileLength();
    // Bucket is empty so far (adding first association)
    if (!b.used) {
        
        // Let header know a new bucket has been added
        Header h;
        if (!m_file.read(h, 0)) cerr << "Failed to read from disk!" << endl;
        h.bucketsUsed++;
        if (!m_file.write(h, 0)) cerr << "Failed to write to disk!" << endl;
        
        a.next = NULL_BUCKET;                   // no next value
        b.used = true;
        
        strcpy(b.key, key.c_str()); // copy key into the bucket
    
        if (!m_file.write(a, insertHere)) cerr << "Failed to write to disk!" << endl;   // write to end of file (for now)
        
        // ------------------------------------- //
        // TODO: Modify this when reusing spaces //
        // ------------------------------------- //
    }
    
    // Bucket already has an association
    else {
        a.next = b.head;  // set new association's next to bucket's current head
        
        if (!m_file.write(a, insertHere)) cerr << "Failed to write to disk!" << endl;;
    }
    
    // Set the bucket's head to be newly inserted association
    b.head = insertHere;
    if (!m_file.write(b, bucketOffset)) cerr << "Failed to write to disk!" << endl;;
    
    close();
    return true;
}

// BOR: O(N/B) or O(K), Actual: O(1)
DiskMultiMap::Iterator DiskMultiMap::search(const std::string& key) {

    openExisting(m_filename);
    
    // Read the appropriate bucket from the hashing function
    Bucket b;
    if (!m_file.read(b, keyHasher(key))) cerr << "Failed to read from disk!" << endl;; // ERROR: Not finding the correct bucket
    
    // Create an iterator pointing to first association of that bucket
    if (b.used) {
        DiskMultiMap::Iterator it(m_filename, b.head, key, ASSOCIATION_SIZE - sizeof(BinaryFile::Offset));
        close();
        return it;
    }
    // If no association, return an invalid iterator
    else {
        DiskMultiMap::Iterator it;
        close();
        return it;
    }
}

// BOR: O(N/B) or O(K), Actual: O(K)
int DiskMultiMap::erase(const std::string& key, const std::string& value, const std::string& context) {
    
    openExisting(m_filename);
    
    // Read the appropriate bucket from the hashing function
    Bucket b;
    m_file.read(b, keyHasher(key));
    
    // Key has no associations
    if (!b.used) {
        close();
        return 0;
    }
    
    // Start at first item with that key
    BinaryFile::Offset currAssociation = b.head;
    
    int deletedCount = 0;
    Association firstA;
    m_file.read(firstA, currAssociation);
    string firstValue = firstA.value;
    string firstContext = firstA.context;
    
    // Check if first node needs to be deleted
    if (firstValue == value && firstContext == context) {
        // Save node's location in a temp (to be reused)
        // Set bucket's head to this node's next
        
        b.head = firstA.next;
        
        m_file.write(b, keyHasher(key));
        currAssociation = b.head;
        
        deletedCount++;
    }
    
    // Look through all the associations for that key
    while (currAssociation != NULL_ASSOC) {
        
        // For a given association, check the next association
        // If it is the association to be deleted,
        // set the current association's "next" to next's next
        // Increment deletedCount;
        
        Association currA;
        m_file.read(currA, currAssociation);

        Association nextA;
        m_file.read(nextA, currA.next);
        
        string nextValue = nextA.value;
        string nextContext = nextA.context;
        if (nextValue == value && nextContext == context) {
            // Found a node to be deleted
            currA.next = nextA.next;
            
            m_file.write(currA, currAssociation);
            // TODO: MARK DELETED NODES AS USABLE TO ADD NEW NODES
            // • "temp" variable above contains freed slot
            
            deletedCount++;
            
            if (currA.next == NULL_ASSOC)
                currAssociation = NULL_ASSOC;
        } else
            currAssociation = currA.next;
    }
    
    // Bucket has emptied from deletion, set it to not used
    if (b.head == NULL_BUCKET) {
        b.used = false;
        m_file.write(b, keyHasher(key));
        
        Header h;
        m_file.read(h, 0);
        h.bucketsUsed++;
        m_file.write(h, 0);
    }

    close();
    return deletedCount;
}

// ----------------------------- //
// ----- PRIVATE FUNCTIONS ----- //
// ----------------------------- //

BinaryFile::Offset DiskMultiMap::keyHasher(const std::string& key) const {
    std::hash<string> hasher;
    size_t hashValue = hasher(key);
    return HEADER_SIZE + (hashValue % m_totalBuckets) * BUCKET_SIZE;
}





