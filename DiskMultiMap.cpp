#include "DiskMultiMap.h"
#include "BinaryFile.h"
#include <functional>

// ------------------------------ //
// ----- ITERATOR FUNCTIONS ----- //
// ------------------------------ //

DiskMultiMap::Iterator::Iterator() {
    m_valid = false;
}

DiskMultiMap::Iterator::Iterator(DiskMultiMap* map, std::string key) {
    m_valid = true;
    m_map = map;
    m_key = key;
    m_gapSize = m_map->ASSOCIATION_SIZE - sizeof(BinaryFile::Offset);
    
    Bucket b;
    m_map->m_file.read(b, m_map->keyHasher(key));
    m_curr = b.head;
}

bool DiskMultiMap::Iterator::isValid() const {
    return m_valid;
}

DiskMultiMap::Iterator& DiskMultiMap::Iterator::operator++() {
    
    
    
    Association a;
    if (!m_map->m_file.read(a, m_curr)) cerr << "Failed to read from disk!" << endl;
    m_curr = a.next;
    if (m_curr == NULL_BUCKET)
        m_valid = false;
    
    return *this;
}

MultiMapTuple DiskMultiMap::Iterator::operator*() {
    
    char value[MAX_WORD_LENGTH + 1];
    char context[MAX_WORD_LENGTH + 1];
    
    // Read the value and context from the current association
    if (!m_map->m_file.read(value, MAX_WORD_LENGTH + 1, m_curr)) cerr << "Failed to read from disk!" << endl;
    if (!m_map->m_file.read(context, MAX_WORD_LENGTH + 1, m_curr + (MAX_WORD_LENGTH + 1))) cerr << "Failed to read from disk!" << endl;
    
    // Fill up the MultiMapTuple to be returned
    MultiMapTuple m;
    m.key = m_key;
    m.value = value;
    m.context = context;
    
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
    h.freeSlotsHead = NULL_ASSOC;
    if (!m_file.write(h, 0)) cerr << "Failed to write to disk!" << endl;
    
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
    
    // Hash the key to find which bucket to insert into
    BinaryFile::Offset bucketOffset = keyHasher(key);
    
    // Read the bucket
    Bucket b;
    if (!m_file.read(b, bucketOffset)) cerr << "Failed to read from disk!" << endl;
    
    // Create first association to insert
    Association a;
    strcpy(a.value, value.c_str());         // copy value into association
    strcpy(a.context, context.c_str());     // copy context into association
    
    BinaryFile::Offset insertHere;
    bool reusedSlot = reuseSlot(insertHere);
    if (reusedSlot) {
        oneSlotReused();
        cerr << "Reused a slot" << endl;
    }
    
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
    }
    
    // Bucket already has an association
    else {
        a.next = b.head;  // set new association's next to bucket's current head
        
        if (!m_file.write(a, insertHere)) cerr << "Failed to write to disk!" << endl;;
    }
    
    // Set the bucket's head to be newly inserted association
    b.head = insertHere;
    if (!m_file.write(b, bucketOffset)) cerr << "Failed to write to disk!" << endl;;
    
    return true;
}

// BOR: O(N/B) or O(K), Actual: O(1)
DiskMultiMap::Iterator DiskMultiMap::search(const std::string& key) {
    
    // Read the appropriate bucket from the hashing function
    Bucket b;
    if (!m_file.read(b, keyHasher(key))) cerr << "Failed to read from disk!" << endl;; // ERROR: Not finding the correct bucket
    
    // Create an iterator pointing to first association of that bucket
    if (b.used) {
        DiskMultiMap::Iterator it(this, key);
        return it;
    }
    // If no association, return an invalid iterator
    else {
        DiskMultiMap::Iterator it;
        return it;
    }
}

// BOR: O(N/B) or O(K), Actual: O(K)
int DiskMultiMap::erase(const std::string& key, const std::string& value, const std::string& context) {
    
    // Read the appropriate bucket from the hashing function
    Bucket b;
    if (!m_file.read(b, keyHasher(key))) cerr << "Failed to read from disk!" << endl;
    
    // Key has no associations
    if (!b.used) return 0;
    
    // Start at first item with that key
    BinaryFile::Offset currOffset = b.head;
    
    int deletedCount = 0;
    bool deletedFirst = false;
    
    // Delete first node if it matches, then check (new) first node again
    do {
        // Get first association
        Association firstA;
        if (!m_file.read(firstA, currOffset)) cerr << "Failed to read from disk!" << endl;
        
        // Check if first node needs to be deleted
        if (firstA.value == value && firstA.context == context) {

            deletedCount++;
            deletedFirst = true;
            
            // Mark bucket's first association as a freed slot
            addToFreeSlotList(b.head);
            
            // Point bucket to second association (aka delete first node)
            b.head = firstA.next;
            if (!m_file.write(b, keyHasher(key))) cerr << "Failed to write to disk!" << endl;
            
            // Update current offset to be the new bucket head
            currOffset = b.head;
        } else {
            deletedFirst = false;
        }
        
    } while (deletedFirst && currOffset != NULL_ASSOC);
    
    // Look through all the associations for that key
    while (currOffset != NULL_ASSOC) {
        
        // Read the current and next association (if there is a next)
        Association currA, nextA;
        if (!m_file.read(currA, currOffset)) cerr << "Failed to read from disk!" << endl;
        if (currA.next != -1) {
            if (!m_file.read(nextA, currA.next)) cerr << "Failed to read from disk!" << endl;
        }
        else    // Nothing left to delete
            return deletedCount;
        
        // If the next association is to be deleted
        if (nextA.value == value && nextA.context == context) {
            
            // Mark next association as deleted
            addToFreeSlotList(currA.next);
            deletedCount++;
            
            // Set current association to point to deleted association's next
            currA.next = nextA.next;
            if (!m_file.write(currA, currOffset)) cerr << "Failed to read from disk!" << endl;
            
            // If the next association is empty (only one left in list)
            // Done deleting
            if (currA.next == NULL_ASSOC)
                currOffset = NULL_ASSOC;
        } else
            currOffset = currA.next;    // Move offset to next offset
    }
    
    // Bucket has emptied from deletion, set it to not used
    if (b.head == NULL_ASSOC) {
        b.used = false;
        if (!m_file.write(b, keyHasher(key))) cerr << "Failed to write to disk!" << endl;
        
        Header h;
        if (!m_file.read(h, 0)) cerr << "Failed to read to disk!" << endl;
        h.bucketsUsed--;
        if (!m_file.write(h, 0)) cerr << "Failed to write to disk!" << endl;
    }
    return deletedCount;
}

// FOR TESTING ONLY: DELETE AFTER TESTING!!!!!!! //

BinaryFile::Offset DiskMultiMap::fileLength() { return m_file.fileLength(); }

// ----------------------------- //
// ----- PRIVATE FUNCTIONS ----- //
// ----------------------------- //

BinaryFile::Offset DiskMultiMap::keyHasher(const std::string& key) const {
    std::hash<string> hasher;
    size_t hashValue = hasher(key);
    return HEADER_SIZE + (hashValue % m_totalBuckets) * BUCKET_SIZE;
}

void DiskMultiMap::addToFreeSlotList(BinaryFile::Offset slot) {
    Header h;
    if (!m_file.read(h, 0)) cerr << "Failed to read from disk!" << endl;
    
    // No free slots currently
    if (h.freeSlotsHead == NULL_ASSOC) {
        Association a;
        m_file.read(a, slot);
        a.next = NULL_ASSOC;
        m_file.write(a, slot);
    }
    else {
        // Push new empty slot to top of freeSlots list
        
        // Set new slot's next to be current top empty slot
        Association a;
        if (!m_file.read(a, slot)) cerr << "Failed to read from disk!" << endl;
        a.next = h.freeSlotsHead;
        if (!m_file.write(a, slot)) cerr << "Failed to read from disk!" << endl;
    }
    
    h.freeSlotsHead = slot;
    if (!m_file.write(h, 0)) cerr << "Failed to write to disk!" << endl;
}

bool DiskMultiMap::reuseSlot(BinaryFile::Offset& slot) {
    
    Header h;
    if (!m_file.read(h, 0)) cerr << "Failed to read from disk!" << endl;
    BinaryFile::Offset freeSlot = h.freeSlotsHead;
    
    if (freeSlot == NULL_ASSOC) {
        slot = m_file.fileLength();
        return false;
    }
    else {
        slot = h.freeSlotsHead;
        return true;
    }
}

void DiskMultiMap::oneSlotReused() {
    
    // Set freeSlotsHead to point to the head after the current first free slot
    // Because that slot (presumably) just got filled up
    
    Header h;
    if (!m_file.read(h, 0)) cerr << "Failed to read from disk!" << endl;
    
    Association a;
    if (!m_file.read(a, h.freeSlotsHead)) cerr << "Failed to read from disk!" << endl;
    
    h.freeSlotsHead = a.next;
    if (!m_file.write(h, 0)) cerr << "Failed to write to disk!" << endl;
}