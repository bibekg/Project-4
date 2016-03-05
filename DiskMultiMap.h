#ifndef DISKMULTIMAP_H_
#define DISKMULTIMAP_H_

#include <string>
#include "MultiMapTuple.h"
#include "BinaryFile.h"

class DiskMultiMap
{
public:
    
    class Iterator
    {
    public:
        Iterator();
        // You may add additional constructors
        bool isValid() const;
        Iterator& operator++();
        MultiMapTuple operator*();
        
    private:
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
    
private:
    
    const int HEADER_SIZE = sizeof(Header);
    const int BUCKET_SIZE = sizeof(Bucket);
    
    BinaryFile m_file;
    std::string m_filename;
    
    struct Header {
        int totalBuckets;
        int bucketsUsed;
        // member variable to keep track of
        // empty spots available for reuse
    };
    
    struct Bucket {
        MultiMapTuple data;
        BinaryFile::Offset next;
        bool used;
    };
};

#endif // DISKMULTIMAP_H_