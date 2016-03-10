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
    
    // Read bucket
    Bucket b;
    if (!m_map->m_file.read(b, m_map->keyHasher(m_key))) cerr << "Failed to read from disk!" << endl;
    
    m_curr = b.head;
    
    // Read first association in bucket
    Association a;
    if (!m_map->m_file.read(a, m_curr)) cerr << "Failed to read from disk!" << endl;
    
    // If association's key doesn't match, move it to first one that does
    if (a.key != m_key)
        operator++();
}

bool DiskMultiMap::Iterator::isValid() const {
    return m_valid;
}

DiskMultiMap::Iterator& DiskMultiMap::Iterator::operator++() {
    
    Association a, c;
    
    // Read next until you find a matching key or hit the end
    do {
        // Read the current association
        if (!m_map->m_file.read(a, m_curr)) cerr << "Failed to read from disk!" << endl;
        
        // Increment to next association
        m_curr = a.next;
        
        //
        if (m_curr != NULL_SLOT) {
            // Read the current association
            if (!m_map->m_file.read(c, m_curr)) cerr << "Failed to read from disk!" << endl;
            if (c.key == m_key)
                break;
        }

    } while (c.key != m_key && m_curr != NULL_SLOT);

    // If the iterator hit the end, set it to invalid
    if (m_curr == NULL_SLOT)
        m_valid = false;
    
    return *this;
}

MultiMapTuple DiskMultiMap::Iterator::operator*() {
    
    // Read the value and context from the current association
    Association a;
    m_map->m_file.read(a, m_curr);
    
    // Fill up the MultiMapTuple to be returned
    MultiMapTuple m;
    m.key = a.key;
    m.value = a.value;
    m.context = a.context;
    
    return m;
}

// ---------------------------------- //
// ----- DISKMULTIMAP FUNCTIONS ----- //
// ---------------------------------- //

DiskMultiMap::DiskMultiMap() {
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
    h.freeSlotsHead = NULL_SLOT;
    if (!m_file.write(h, 0)) cerr << "Failed to write to disk!" << endl;
    
    // Create #numBuckets buckets
    // Runs in O(numBuckets) time
    for (int i = 0; i < numBuckets; i++) {
        Bucket b;
        b.used = false;
        b.head = NULL_SLOT;
        
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
    if (m_file.isOpen()) m_file.close();
}

// BOR: O(N/B), Actual: O(1)
// TODO: MODIFY TO ALLOW INSERTING INTO GAPS
bool DiskMultiMap::insert(const std::string& key, const std::string& value, const std::string& context) {
    
    // Input is too long, return false immediately
    if (key.length() > MAX_WORD_LENGTH || value.length() > MAX_WORD_LENGTH || context.length() > MAX_WORD_LENGTH)
        return false;
    
    // Read the bucket
    Bucket b;
    if (!m_file.read(b, keyHasher(key))) cerr << "Failed to read from disk!" << endl;
    
    // Create first association to insert
    Association a;
    strcpy(a.key, key.c_str());             
    strcpy(a.value, value.c_str());         
    strcpy(a.context, context.c_str());
    
    // Determine where to insert the association
    BinaryFile::Offset insertHere;
    if (reuseSlot(insertHere))
        oneSlotReused();
    
    // Bucket is empty so far (adding first association)
    if (!b.used) {
        b.used = true;
        a.next = NULL_SLOT;
    
        if (!m_file.write(a, insertHere)) cerr << "Failed to write to disk!" << endl;
    }
    
    // Bucket already has an association
    else {
        a.next = b.head;  // set new association's next to bucket's current head
        
        if (!m_file.write(a, insertHere)) cerr << "Failed to write to disk!" << endl;;
    }
    
    // Set the bucket's head to be newly inserted association
    b.head = insertHere;
    if (!m_file.write(b, keyHasher(key))) cerr << "Failed to write to disk!" << endl;;
    
    return true;
}

// BOR: O(N/B) or O(K), Actual: O(1)
DiskMultiMap::Iterator DiskMultiMap::search(const std::string& key) {
    
    // Read the appropriate bucket from the hashing function
    Bucket b;
    if (!m_file.read(b, keyHasher(key))) cerr << "Failed to read from disk!" << endl;; // ERROR: Not finding the correct bucket
    
    if (b.used) {
        // Create an iterator pointing to first association of that key
        DiskMultiMap::Iterator it(this, key);
        
        // Resulting iterator may be invalid if that bucket did not hold
        // an association with the relevant key
        return it;
    }
    // The key does not exist, return an invalid iterator
    else {
        DiskMultiMap::Iterator it;
        return it;
    }
}

// BOR: O(N/B) or O(K), Actual: O(K)
int DiskMultiMap::erase(const std::string& key, const std::string& value, const std::string& context) {
    
    // I want to erase every association that matches all three criterion: key, value, context.
    
    // Read the appropriate bucket from the hashing function
    Bucket b;
    if (!m_file.read(b, keyHasher(key))) cerr << "Failed to read from disk!" << endl;
    
    // Key has no associations
    if (!b.used) return 0;
    
    // Start at first item with that key
    BinaryFile::Offset currOffset = b.head;
    
    int deletedCount = 0;
    bool deletedFirst = false;
    
    // Check if first node needs to be deleted
    do {
        Association firstA;
        if (!m_file.read(firstA, currOffset)) cerr << "Failed to read from disk!" << endl;
        
        if (firstA.key == key && firstA.value == value && firstA.context == context) {

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
        
    } while (deletedFirst && currOffset != NULL_SLOT);
    // Repeat until the first node was not deleted or the bucket is empty
    
    // Look through all the associations for that key
    while (currOffset != NULL_SLOT) {
        
        // Read the current association
        Association currA, nextA;
        if (!m_file.read(currA, currOffset)) cerr << "Failed to read from disk!" << endl;
        
        // Read the next association
        if (currA.next == NULL_SLOT)
            return deletedCount;    // nothing left to delete
        else if (!m_file.read(nextA, currA.next)) cerr << "Failed to read from disk!" << endl;
        
        // If the next association is to be deleted
        if (nextA.key == key && nextA.value == value && nextA.context == context) {
            
            // Mark next association as deleted
            addToFreeSlotList(currA.next);
            deletedCount++;
            
            // Set current association to point to deleted association's next
            currA.next = nextA.next;
            if (!m_file.write(currA, currOffset)) cerr << "Failed to read from disk!" << endl;
            
            // If the next association is empty, we're done
            if (currA.next == NULL_SLOT)
                break;
            
            // Recheck from currA since nextA has changed
        } else
            currOffset = currA.next;    // Move offset to next offset
    }
    
    // Bucket has emptied from deletion, set it to not used
    if (b.head == NULL_SLOT) {
        b.used = false;
        if (!m_file.write(b, keyHasher(key))) cerr << "Failed to write to disk!" << endl;
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
    if (h.freeSlotsHead == NULL_SLOT) {
        Association a;
        m_file.read(a, slot);
        a.next = NULL_SLOT;
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
    
    if (freeSlot == NULL_SLOT)          // No reusable slots, append to back
        slot = m_file.fileLength();
    else                                // Reuse slot
        slot = h.freeSlotsHead;

    return slot == h.freeSlotsHead;     // True if slot was reused
}

void DiskMultiMap::oneSlotReused() {
    Header h;
    Association a;
    if (!m_file.read(h, 0)) cerr << "Failed to read from disk!" << endl;
    if (!m_file.read(a, h.freeSlotsHead)) cerr << "Failed to read from disk!" << endl;
    
    // Update freeSlotsHead to next free slot
    h.freeSlotsHead = a.next;
    if (!m_file.write(h, 0)) cerr << "Failed to write to disk!" << endl;
}