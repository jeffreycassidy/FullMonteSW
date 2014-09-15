#include <string>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <array>

#include <cstring>

#include <vector>

#include <boost/static_assert.hpp>

#include <boost/numeric/ublas/matrix.hpp>

using namespace std;

inline size_t padBytes(size_t Nbytes,size_t Npad)
{
	return (Npad - (Nbytes%Npad)) % Npad;
}

inline size_t paddedLength(size_t Nbytes,size_t Npad)
{
	return Nbytes + padBytes(Nbytes,Npad);
}


inline void writePadded8B(ostream& os,const void* data,size_t Nbytes)
{
	uint8_t pad[8]={0,0,0,0,0,0,0,0};
	os.write((const char*)data,Nbytes);

	// pad up to a multiple of 8 bytes
	os.write((const char*)pad,padBytes(Nbytes,8));
}

void writeSubElement(ostream& os,const string& str);

namespace MatFile {
	enum miDataType {
		miINT8=1,
		miUINT8=2,
		miINT16=3,
		miUINT16=4,
		miINT32=5,
		miUINT32=6,
		miSINGLE=7,
		miDOUBLE=9,
		miINT64=12,
		miUINT64=13,
		miMATRIX=14,
		miCOMPRESSED=15,
		miUTF8=16,
		miUTF16=17,
		miUTF32=18
	};

	enum miArrayType {
		mxCELL_CLASS=1,
		mxSTRUCT_CLASS=2,
		mxOBJECT_CLASS=3,
		mxCHAR_CLASS=4,
		mxSPARSE_CLASS=5,
		mxDOUBLE_CLASS=6,
		mxSINGLE_CLASS=7,
		mxINT8_CLASS=8,

		mxUINT8_CLASS=9,
		mxINT16_CLASS=10,
		mxUINT16_CLASS=11,
		mxINT32_CLASS=12,
		mxUINT32_CLASS=13,
		mxINT64_CLASS=14,
		mxUINT64_CLASS=15
	};

	// packed attribute is required to ensure correspondence with byte structure in file
	class __attribute__((__packed__)) Header {
	public:
		uint8_t  text[116];
		uint64_t offs=0;
		uint16_t ver=0x0100;
		uint16_t endian=('M'<<8) | 'I';				// endian check: will write as "IM" on LE machines (eg Intel)

		Header(const char* text_ = "%%%% Written by MatFile.cpp (default message)")
			{ strncpy((char*)text,text_,116); }

		Header(string text_)
			{ strncpy((char*)text,text_.c_str(),116); };
	};

	template<typename T>struct type_traits{};

	template<>struct type_traits<char> {
		typedef unsigned char type;
		static const unsigned bytesize=1;
		static const miDataType datatype=miINT8;
		static const miArrayType array_class=mxINT8_CLASS;
	};

	template<>struct type_traits<unsigned char> {
		typedef unsigned char type;
		static const unsigned bytesize=1;
		static const miDataType datatype=miUINT8;
		static const miArrayType array_class=mxUINT8_CLASS;
	};

	template<>struct type_traits<int32_t> {
		typedef int32_t type;
		static const unsigned bytesize=4;
		static const miDataType datatype=miINT32;
		static const miArrayType array_class=mxINT32_CLASS;
	};

	template<>struct type_traits<uint32_t> {
		typedef uint32_t type;
		static const unsigned bytesize=4;
		static const miDataType datatype=miUINT32;
		static const miArrayType array_class=mxUINT32_CLASS;
	};

	template<>struct type_traits<uint64_t> {
		typedef uint64_t type;
		static const unsigned bytesize=8;
		static const miDataType datatype=miUINT64;
		static const miArrayType array_class=mxUINT64_CLASS;
	};

	template<>struct type_traits<double> {
		typedef double type;
		static const unsigned bytesize=8;
		static const miDataType datatype=miDOUBLE;
		static const miArrayType array_class=mxDOUBLE_CLASS;
	};

	typedef struct __attribute__((__packed__)) {
		uint32_t datatype;
		uint32_t datasize;
	} ElementTag;
}


template<class T>void writeSubElement(ostream& os,const T* data,unsigned Nel=1)
{
	size_t bytes=MatFile::type_traits<T>::bytesize*Nel;
	MatFile::ElementTag etag = { MatFile::type_traits<T>::datatype, paddedLength(bytes,8) };
	os.write((const char*)&etag,sizeof(etag));

	writePadded8B(os,data,bytes);

	BOOST_STATIC_ASSERT(sizeof(etag)==8);
}

void writeSubElement(ostream& os,const string& str);

template<class T,size_t N>void writeSubElement(ostream& os,const array<T,N>& data)
{
	writeSubElement(os,data.data(),N);
}

template<class T>void writeSubElement(ostream& os,T data)
{
	writeSubElement(os,&data,1);
}



template<class T>void writeElement(ostream& os,string name,const vector<T>& v)
{
	// includes all subelements: flags (16), dims (8 + 4D), name (8 + L), data (8+BN)
	MatFile::ElementTag etag{ MatFile::miMATRIX, 16+16+paddedLength(8+name.length(),8)+paddedLength(8+v.size()*MatFile::type_traits<T>::bytesize,8)};
	os.write((const char*)&etag,8);

	// array flags subelement
	bool cplx=false;
	bool global=false;
	bool logical=false;
	uint32_t arrclass=MatFile::type_traits<T>::array_class;

	array<uint32_t,2> arrflags;
	arrflags[0] = (cplx << 19) | (global << 18) | (logical << 17) | (arrclass << 24);
	arrflags[1] = 0;

	writeSubElement(os,arrflags);

	// dimensions array
	array<int32_t,2> dims{v.size(),1};
	writeSubElement(os,dims);

	// array name
	writeSubElement(os,name);

	// array values
	writeSubElement(os,v.data(),v.size());
}

template<class T,class Iterator>void writeSubElements(ostream& os,Iterator begin,Iterator end,int size=-1)
{
	for(;begin != end; ++begin)
		os.write(&(*begin),sizeof(T));
}

template<class T>void writeElement(ostream& os,string name,const typename boost::numeric::ublas::matrix<T,boost::numeric::ublas::column_major>& M)
{
	// includes all subelements: flags (16), dims (8 + 4D), name (8 + L), data (8+BN)
	MatFile::ElementTag etag{ MatFile::miMATRIX, 16+16+paddedLength(8+name.length(),8)+paddedLength(8+M.size1()*M.size2()*MatFile::type_traits<T>::bytesize,8)};
	os.write((const char*)&etag,8);

	// array flags subelement
	bool cplx=false;
	bool global=false;
	bool logical=false;
	uint32_t arrclass=MatFile::type_traits<T>::array_class;

	array<uint32_t,2> arrflags;
	arrflags[0] = (cplx << 19) | (global << 18) | (logical << 17) | (arrclass << 24);
	arrflags[1] = 0;

	writeSubElement(os,arrflags);

	// dimensions array
	array<int32_t,2> dims{M.size1(),M.size2()};
	writeSubElement(os,dims);

	// array name
	writeSubElement(os,name);

	//TODO: Fix padding - currently none; will not always work for types other than double (which is 8 bytes)
	MatFile::ElementTag etag2 { MatFile::miDOUBLE, M.size1()*M.size2()*sizeof(T) };
	os.write((const char*)&etag2,8);

	typedef typename boost::numeric::ublas::matrix<T,boost::numeric::ublas::column_major>::const_iterator1 RowIterator;
	typedef typename boost::numeric::ublas::matrix<T,boost::numeric::ublas::column_major>::const_iterator2 ColIterator;

	// Iterate through array and write values
	for(ColIterator begin_c=M.begin2(),end_c=M.end2(); begin_c != end_c; ++begin_c)
		for(RowIterator begin_r=begin_c.begin(), end_r=begin_c.end(); begin_r != end_r; ++begin_r)
			os.write((const char*)&(*begin_r),MatFile::type_traits<T>::bytesize);
}
