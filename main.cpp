//
//  main.cpp
//  Project 4
//
//  Created by Bibek Ghimire on 3/6/16.
//  Copyright © 2016 Bibek Ghimire. All rights reserved.
//


#include <iostream>
#include <cassert>
#include "DiskMultiMap.h"
#include "MultiMapTuple.h"

using namespace std;

void reuseTests() {
    DiskMultiMap x;
    x.createNew("myhashtable.dat",100); // empty, with 100 buckets
    cerr << "Initial size: " << x.fileLength() << endl;
    
    x.insert("hmm.exe", "pfft.exe", "m52902");
    x.insert("hmm.exe", "pfft.exe", "m52902");
    x.insert("hmm.exe", "pfft.exe", "m52902");
    cerr << "After 3 inserts: " << x.fileLength() << endl;
    
    BinaryFile::Offset initlength = x.fileLength();
    
    x.erase("hmm.exe", "pfft.exe", "m52902");
    cerr << "After erase: " << x.fileLength() << endl;
    
    x.insert("hmm.exe", "pfft.exe", "m52902");
    x.insert("hmm.exe", "pfft.exe", "m52902");
    x.insert("hmm.exe", "pfft.exe", "m52902");
    cerr << "After 3 more inserts: " << x.fileLength() << endl;
    
    assert(initlength == x.fileLength());
    
    x.insert("hmm.exe", "pfft.exe", "m52902");
    cerr << "After one extra insert: " << x.fileLength() << endl;
    x.insert("hmm.exe", "pfft.exe", "m52902");
    cerr << "After two extra inserts: " << x.fileLength() << endl;
    x.insert("hmm.exe", "pfft.exe", "m52902");
    cerr << "After three extra inserts: " << x.fileLength() << endl;
    
    BinaryFile::Offset postLength = x.fileLength();
    assert(initlength != postLength);
    
    cout << "passed" << endl;
    
}

void iteratorTests() {
    
    DiskMultiMap x;
    x.createNew("myhashtable.dat",100); // empty, with 100 buckets
    
    x.insert("hmm.exe", "pfft.exe", "m52902");
    x.insert("foo.exe", "pfft.exe", "m44444");
    
    DiskMultiMap::Iterator it = x.search("hmm.exe");
    assert(it.isValid());
    
    x.erase("hmm.exe", "pfft.exe", "m52902");
    it = x.search("hmm.exe");
    assert(!it.isValid());
    
    x.erase("foo.exe", "pfft.exe", "m44444");
    it = x.search("foo.exe");
    assert(!it.isValid());
    
    DiskMultiMap::Iterator it2 = x.search("hmm.exe");
    assert(!it.isValid());
    
    cout << "👍🏼" << endl;
}

void iteratorTests2() {
    DiskMultiMap x;
    x.createNew("myhashtable.dat",100); // empty, with 100 buckets
    x.insert("hmm.exe", "pfft.exe", "m52902");
    x.insert("hmm.exe", "pfft.exe", "m52902");
    x.insert("hmm.exe", "pfft.exe", "m10001");
    x.insert("blah.exe", "bletch.exe", "m0003");
    DiskMultiMap::Iterator it = x.search("hmm.exe");
    if (!it.isValid())
        cout << "I couldn’t find goober.exe\n";
    
    if (it.isValid()) {
        cout << "I found at least 1 item with a key of hmm.exe\n";
        do {
            MultiMapTuple m = *it; // get the association
            cout << "The key is: " << m.key << endl;
            cout << "The value is: " << m.value << endl;
            cout << "The context is: " << m.context << endl;
            cout << endl;
            ++it; // advance iterator to the next matching item
        } while (it.isValid());
    }
}

int main() {
    
}