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

//int main() {
//    DiskMultiMap x;
//    x.createNew("myhashtable.dat",100); // empty, with 100 buckets
//    x.insert("hmm.exe", "pfft.exe", "m52902");
//    x.insert("hmm.exe", "pfft.exe", "m52902");
//    x.insert("hmm.exe", "pfft.exe", "m10001");
//    x.insert("blah.exe", "bletch.exe", "m0003");
//    DiskMultiMap::Iterator it = x.search("goober.exe");
//    if (!it.isValid())
//        cout << "I couldn’t find goober.exe\n";
//    
////    if (it.isValid()) {
////        cout << "I found at least 1 item with a key of hmm.exe\n";
////        do {
////            MultiMapTuple m = *it; // get the association
////            cout << "The key is: " << m.key << endl;
////            cout << "The value is: " << m.value << endl;
////            cout << "The context is: " << m.context << endl;
////            cout << endl;
////            ++it; // advance iterator to the next matching item
////        } while (it.isValid());
////    }
//}
//
////int main() {
////    cout << sizeof(char[121]) + sizeof(char[121]) + sizeof(BinaryFile::Offset);
////}

int main() {
    DiskMultiMap x;
    x.createNew("myhashtable.dat",100); // empty, with 100 buckets
    
    x.insert("hmm.exe", "pfft.exe", "m52902");
    x.insert("hmm.exe", "pfft.exe", "m52902");
    x.insert("hmm.exe", "pfft.exe", "m52902");
    x.insert("hmm.exe", "pfft.exe", "m52902");
    
    int initlength = x.fileLength();
    
    x.erase("hmm.exe", "pfft.exe", "m52902");
    
    x.insert("hmm.exe", "pfft.exe", "m52902");
    x.insert("hmm.exe", "pfft.exe", "m52902");
    x.insert("hmm.exe", "pfft.exe", "m52902");
    
    assert(initlength == x.fileLength());
    
    cout << "passed" << endl;
    
//    x.insert("hmm.exe", "pfft.exe", "m52902");
//    x.insert("foo.exe", "pfft.exe", "m44444");
//    
//    DiskMultiMap::Iterator it = x.search("hmm.exe");
//    assert(it.isValid());
//    
//    x.erase("hmm.exe", "pfft.exe", "m52902");
//    it = x.search("hmm.exe");
//    assert(!it.isValid());
//    
//    x.erase("foo.exe", "pfft.exe", "m44444");
//    it = x.search("foo.exe");
//    assert(!it.isValid());
//    
//    DiskMultiMap::Iterator it2 = x.search("hmm.exe");
//    assert(!it.isValid());
//    
//    cout << "👍🏼" << endl;
//    
//    
//    x.insert("hmm.exe", "pfft.exe", "m52902");
//    x.insert("hmm.exe", "pfft.exe", "m52902");
//    x.insert("hmm.exe", "pfft.exe", "m10001");
//    x.insert("blah.exe", "bletch.exe", "m0003");
//    
//    // line 1
//    if (x.erase("hmm.exe", "pfft.exe", "m52902") == 2)
//        cout << "Just erased 2 items from the table!\n";
//    // line 2
//    if (x.erase("hmm.exe", "pfft.exe", "m10001") > 0)
//        cout << "Just erased at least 1 item from the table!\n";
//    // line 3
//    if (x.erase("blah.exe", "bletch.exe", "m66666") == 0)
//        cout << "I didn't erase this item cause it wasn't there\n";
//    
//
//    // -------------
//    cout << endl;
//    
//    
//    x.insert("hmm.exe", "pfft.exe", "m52902");
//    x.insert("hmm.exe", "pfft.exe", "m52902");
//    x.insert("hmm.exe", "pfft.exe", "m10001");
//    x.insert("blah.exe", "bletch.exe", "m0003");
//    it = x.search("hmm.exe");
//    if (!it.isValid())
//        cout << "I couldn’t find goober.exe\n";
//    
//    if (it.isValid()) {
//        cout << "I found at least 1 item with a key of hmm.exe\n";
//        do {
//            MultiMapTuple m = *it; // get the association
//            cout << "The key is: " << m.key << endl;
//            cout << "The value is: " << m.value << endl;
//            cout << "The context is: " << m.context << endl;
//            cout << endl;
//            ++it; // advance iterator to the next matching item
//        } while (it.isValid());
//    }

}