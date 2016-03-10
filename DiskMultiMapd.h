#ifndef DISKMULTIMAP_H_
#define DISKMULTIMAP_H_

// Implementation of DiskMultiMap using in-memory data structures until
// the DiskMultiMap is closed or destroyed, at which point the data is
// saved to disk as a text file, not a BinaryFile.

// This is provided solely so that you can test IntelWeb even if you are not
// confident that your implementation of DiskMultiMap is correct.  Of course,
// since it uses in-memory data structures to hold all the data, the spec
// forbids you from using this as the DiskMultiMap implementation you turn in.

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <list>
#include <utility>
#include "MultiMapTuple.h"

class DiskMultiMap
{
public:
	class Iterator
	{
	public:
		Iterator();
		bool isValid() const;
		Iterator& operator++();
		MultiMapTuple operator*();
	private:
        bool v;
        const std::string*kp;
        std::list<std::pair<std::string,std::string>>::const_iterator first;
        std::list<std::pair<std::string,std::string>>::const_iterator second;
    public:Iterator(std::map<std::string,std::list<std::pair<
std::string,std::string>>>::const_iterator p);
    };private:std::string fn;std::
fstream f;std::map<std::string,std::list<std::pair<std::string,std::string>>>m;

public:
	DiskMultiMap();
	~DiskMultiMap();
	bool createNew(const std::string& filename, unsigned int numBuckets);
	bool openExisting(const std::string& filename);
	void close();
	bool insert(const std::string& key, const std::string& value, const std::string& context);
	Iterator search(const std::string& key);
	int erase(const std::string& key, const std::string& value, const std::string& context);
};

#endif // DISKMULTIMAP_H_
