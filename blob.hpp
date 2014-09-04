#pragma once
#include <string>
#include <iostream>
#include <openssl/sha.h>

// TODO: Base64 output iterator
// TODO: Make SHA1 a dynamically-loaded library?
// TODO: Integrate better with Postgres library
// TODO: XML writer?
// TODO: Iterators

using std::string;

string readBinary(string fn);
void writeBinary(const string& fn,const string& s);
string hash_sha1_160(string s);

class SHA1_160_SUM : private string {

public:
	using string::begin;
	using string::end;

	SHA1_160_SUM() : string(20,'\0'){}

	SHA1_160_SUM(const unsigned char* d,unsigned long N){
		SHA1((const uint8_t*)d,N,(uint8_t*)data());
	}

	SHA1_160_SUM(const string& s){
		SHA1((const uint8_t*)s.c_str(),s.size(),(uint8_t*)data());
	}

	bool operator==(const SHA1_160_SUM& rhs) const { return operator==((const string&)rhs); }

	operator const uint8_t*() const { return (const uint8_t*)data(); }
	operator const char*() const { return (const char*)data(); }

	string as_hex() const;

	friend std::ostream& operator<<(std::ostream&,const SHA1_160_SUM&);
};
