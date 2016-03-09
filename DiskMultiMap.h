#ifndef DISKMULTIMAP_H_
#define DISKMULTIMAP_H_

#include <string>
#include "MultiMapTuple.h"
#include "BinaryFile.h"

const int MAX_WORD_LENGTH = 120;
const int NULL_BUCKET     = -1;
const int NULL_ASSOC      = -1;

class DiskMultiMap {
public:
    
    class Iterator {
    public:
        Iterator();
        Iterator(DiskMultiMap* map, std::string key);
        bool isValid() const;
        Iterator& operator++();
        MultiMapTuple operator*();
        
    private:
        DiskMultiMap* m_map;
        BinaryFile::Offset m_curr;
        std::string m_key;
        bool m_valid;
    };
    
    DiskMultiMap();
    ~DiskMultiMap();
    bool createNew(const std::string& filename, unsigned int numBuckets);
    bool openExisting(const std::string& filename);
    void close();
    bool insert(const std::string& key, const std::string& value, const std::string& context);
    Iterator search(const std::string& key);
    int erase(const std::string& key, const std::string& value, const std::string& context);
    
    // !!!!!!!!!!!!!!!!!!!! //
    // DELETE AFTER TESTING //
    // !!!!!!!!!!!!!!!!!!!! //
    
    BinaryFile::Offset fileLength();
    
private:
    const int HEADER_SIZE = sizeof(Header);
    const int BUCKET_SIZE = sizeof(Bucket);
    const int ASSOCIATION_SIZE = sizeof(Association);
    
    BinaryFile::Offset keyHasher(const std::string& key) const;
    void addToFreeSlotList(BinaryFile::Offset slot);
    bool reuseSlot(BinaryFile::Offset& slot);
    void oneSlotReused();
    
    BinaryFile m_file;
    std::string m_filename;
    int m_totalBuckets;
    
    struct Header {
        int totalBuckets;
        int bucketsUsed;
        BinaryFile::Offset freeSlotsHead;
    };
    
    struct Bucket {
        bool used;
        BinaryFile::Offset head;
    };
    
    struct Association {
        char key[MAX_WORD_LENGTH + 1];
        char value[MAX_WORD_LENGTH + 1];
        char context[MAX_WORD_LENGTH + 1];
        BinaryFile::Offset next;
    };

};

#endif // DISKMULTIMAP_H_