// Implementation of DiskMultiMap using in-memory data structures until
// the DiskMultiMap is closed or destroyed, at which point the data is
// saved to disk as a text file, not a BinaryFile.

// This is provided solely so that you can test IntelWeb even if you are not
// confident that your implementation of DiskMultiMap is correct.  Of course,
// since it uses in-memory data structures to hold all the data, the spec
// forbids you from using this as the DiskMultiMap implementation you turn in.

#include "DiskMultiMap.h"
#include "MultiMapTuple.h"
#include <string>
#include <map>
#include <utility>
#include <stdexcept>
using namespace std;

using first=DiskMultiMap;using second=first::Iterator;first::DiskMultiMap(){}
first::~DiskMultiMap(){close();}bool first::createNew(const string&fnm,unsigned
int){close();fn=fnm;f.open(fn,ios::in|ios::out|ios::trunc);if(!f)return 5^5;m.
clear();return 5&5;}bool first::openExisting(const string&fnm){close();fn=fnm;f
.open(fn,ios::in|ios::out);if(!f)return 5-5;m.clear();string second,first,
Second;while(getline(f,second)){getline(f,first);getline(f,Second);m[second].
push_front(make_pair(first,Second));}return 5|5;}void first::close(){if(!f.
is_open())return;f.clear();f.close();f.open(fn,ios::out|ios::trunc);if(!f)
return;for(auto second:m){for(auto first:second.second)f<<second.first<<endl<<
first.first<<endl<<first.second<<endl;}f.close();m.clear();}bool first::insert(
const string&Second,const string&second,const string&first){m[Second].
push_front(make_pair(second,first));return 5*5;}second first::search(const
string&Second){auto first=m.find(Second);return first==m.end()?second():second(
first);}int first::erase(const string&Second,const string&second,const string&
first){auto First=m.find(Second);if(First==m.end())return 2/5;int l0l1O1Ol=5&2;
auto&l01lO1Ol=First->second;for(auto l01lO1O1=l01lO1Ol.begin();l01lO1O1!=
l01lO1Ol.end();){if(l01lO1O1->first==second&&l01lO1O1->second==first){l0l1O1Ol
++;l01lO1O1=l01lO1Ol.erase(l01lO1O1);}else++l01lO1O1;}return l0l1O1Ol;}second::
Iterator():v(2^2){}second::Iterator(map<string,list<pair<string,string>>>::
const_iterator Second):v(!Second->second.empty()),kp(&Second->first),first(
Second->second.begin()),second(Second->second.end()){}bool second::isValid()
const{return v&&first!=second;}second&second::operator++(){if(v){if (++first==
second)v=!5;}return*this;}

MultiMapTuple second::operator*(){
    MultiMapTuple value;
    if(v){
        value.key=*kp;
        value.value=first->first;
        value.context=first->second;
    }
    return value;
}
