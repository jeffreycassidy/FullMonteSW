#include "blob.hpp"
#include <fstream>
#include <sys/stat.h> // for stat/creat
#include <fcntl.h>
#include <sys/types.h>
#include <openssl/sha.h>

using namespace std;

char to_hex_digit(unsigned i)
{
	if (i > 15)
		cerr << "Error in to_hex_digit: out of range" << endl;
	else if (i >= 10)
		return i-10+'A';
	else
		return i+'0';
	return '?';
}

string SHA1_160_SUM::as_hex() const
{
	string s(41,'\0');
	for(unsigned i=0;i<40;++i)
		s[i] = to_hex_digit((array<unsigned char,20>::operator[](i/2) >> ((1-(i%2))<<2)) & 0xf);
	return s;
}

ostream& operator<<(ostream& os,const SHA1_160_SUM& sum)
{
	return os << sum.as_hex();
}

string readBinary(string fn)
{
	// get file size and create string
    struct stat fileStats;

    if(stat(fn.c_str(),&fileStats))
        throw string("Failed to stat() "+fn);

    string s(fileStats.st_size,'\0');

    // read from file, check if valid
    ifstream is(fn.c_str());

    if(!is.good())
        throw string("Failed to open "+fn);

    is.read((char*)s.data(),fileStats.st_size);

    if(is.fail())
        throw string("Failed to read "+fn);
    return s;
}

void writeBinary(const string& fn,const string& s)
{
    ofstream os(fn.c_str());

    if(!os.good())
        throw std::string("Failed to open file for write");

    os.write((const char*)s.data(),s.size());

    if(!os.good())
        throw string("Failed to write");
}
