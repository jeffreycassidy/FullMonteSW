#include "MatFile.hpp"


void writeSubElement(ostream& os,const string& str)
{
	MatFile::ElementTag etag = { MatFile::miINT8, paddedLength(str.length(),8) };
	os.write((const char*)&etag,sizeof(etag));
	BOOST_STATIC_ASSERT(sizeof(etag)==8);

	writePadded8B(os,str.c_str(),str.length());
}

using boost::numeric::ublas::matrix;
using boost::numeric::ublas::column_major;


void writeMatFile(string fn)
{
	ofstream os(fn.c_str());

	MatFile::Header hdr;

	os.write((const char*)&hdr,sizeof(MatFile::Header));
/*
	vector<double> v(5,0.0);

	v[0]=3.141592654;
	v[1]=2.71;
	v[2]=1.0;
	v[3]=0.0;
	v[4]=10.0;

	writeElement(os,"a_vector",v);

	v.resize(1);
	writeElement(os,"foo",v);

	v.resize(3);
	writeElement(os,"smaller_vector_longer_name",v);*/

	matrix<double,column_major> M(5,2);
	unsigned i=0;
	for(unsigned r=0; r<5; ++r)
		for(unsigned c=0; c<2; ++c)
			M(r,c) = i++;

	writeElement(os,"matrix5_2",M);
}


/*
Code for writing structs; does not work yet
{
	// write element tag
	uint32_t datatype=MatFile::miMATRIX;
	uint32_t datasize=0x138;

	os.write((const char*)&datatype,4);
	os.write((const char*)&datasize,4);

	// write array flags
	bool cplx=false, global=false, logical=false;
	array<uint32_t,2> arrflags;

	arrflags[0]=(cplx << 21) | (global << 22) | (logical << 23) | (MatFile::mxSTRUCT_CLASS << 24);
	arrflags[1]=0;

	writeSubElement(os,arrflags);

	// write dims
	array<int32_t,2> dims={1,1};
	writeSubElement(os,dims);

	// struct name
	string arrname("mystruct");
	writeSubElement(os,arrname);

	// field name length
	writeSubElement(os,(uint32_t)16);

	// field names

	datatype=MatFile::miINT8;
	datasize=3*16;
	os.write((const char*)&datatype,4);
	os.write((const char*)&datasize,4);

writeElement(os,"smaller_vector_longer_name",v);
*/
/*
Code for writing structs; does not work yet
{
	// write element tag
	char tmp[16]={"fld0"};
	for(unsigned i=0;i<3;++i)
	{
		tmp[3]=i+'0';
		os.write(tmp,16);
	}

	// write elements

	vector<double> v(1);

	for(unsigned i=0;i<3;++i)
	{
		v[0] = i*100.0;
		writeElement(os,"",v);
	}
}*/
