#include <iostream>
#include "linefile.hpp"
#include "newgeom.hpp"
#include "fluencemap.hpp"

using namespace std;

// define data types for the database (1=surface, 2=volume)
template<>const int FluenceMap<FaceByPointID>::dtype=1;
template<>const int FluenceMap<TetraByPointID>::dtype=2;

template<>const int PointIDLookup<FaceByPointID>::dtype=1;
template<>const int PointIDLookup<TetraByPointID>::dtype=2;

// copies from a vector, squeezing out the zeros
FluenceMapBase::FluenceMapBase(const vector<double>& d,unsigned long long packets_) : packets(packets_)
{
    unsigned i=0;
    for(vector<double>::const_iterator it=d.begin(); it != d.end(); ++it,++i)
        if(*it != 0.0)
            F.insert(make_pair(i,*it));
}

template<>FaceByPointID  PointIDLookup<FaceByPointID>::operator() (unsigned IDf) const { return mesh.getFacePointIDs(IDf);  }
template<>TetraByPointID PointIDLookup<TetraByPointID>::operator()(unsigned IDt) const { return mesh.getTetraPointIDs(IDt); }

template<>unsigned PointIDLookup<FaceByPointID>::operator()(const FaceByPointID& IDps) const
{
    int IDf=mesh.getFaceID(IDps.getSort());
    assert(IDf);
    if (IDf<0)
        return -IDf;
    return IDf;
}

template<>unsigned PointIDLookup<TetraByPointID>::operator()(const TetraByPointID& IDps) const
{
    unsigned IDt=mesh.getTetraID(IDps.getSort());
    assert(IDt);
    return IDt;
}

FluenceMapBase& FluenceMapBase::operator*=(double k)
{
    for(iterator it=F.begin(); it != F.end(); ++it)
        it->second *= k;
    return *this;
}

FluenceMapBase& FluenceMapBase::operator-=(const FluenceMapBase& m)
{
    for(const_iterator it=m.begin(); it != m.end(); ++it)
        operator[](it->first) -= it->second;
    return *this;
}

FluenceMapBase& FluenceMapBase::operator+=(const FluenceMapBase& m)
{
    for(const_iterator it=m.begin(); it != m.end(); ++it)
        operator[](it->first) += it->second;
    return *this;
}

// computes a/b (this / RHS)
FluenceMapBase& FluenceMapBase::operator/=(const FluenceMapBase& m)
{
    const_iterator b=m.begin();

    for (iterator a=F.begin(); a != F.end(); ++a)
    {
        if (b == m.end() || b->first > a->first)
            a->second = std::numeric_limits<double>::infinity();
        else
        {
            for(; b->first < a->first; ++b)
                F.insert(a,make_pair(b->first,0));
            if (a->first == b->first)
            {
                a->second /= b->second;
                ++b;
            }
        }
    }
    return *this;
}

void FluenceMapBase::absdiff()
{
    for(iterator it=F.begin(); it != F.end(); ++it)
        it->second = abs(it->second-1);
}

// serialize the fluence map in binary
string FluenceMapBase::toBinary() const
{
    stringstream ss;

    union {
    	uint32_t id;
    	char p[4];
    } idu;

    union {
    	double val;
    	char p[8];
    } valu;

    double sum=0.0;

    for(pair<unsigned,double> p : F)
    {
    	if (p.second != 0.0)
    	{
    		idu.id=p.first;
    		valu.val =p.second;
    		ss.write(idu.p,4);
    		ss.write(valu.p,8);
    		sum += valu.val;
    	}
    }

    cout << "just wrote a fluencemapbase with total value " << sum << " to database" << endl;

    return ss.str();
}

// convert back from binary format
bool FluenceMapBase::fromBinary(const string& blob)
{
    unsigned N=blob.size();
    const uint8_t *p=blob.data();
    const uint8_t * const p_end = p+N;

    cout << "Blob has " << N << " bytes (" << p_end-p << " ptrdiff), " << N/12 << " elements" << endl;

    if (N % 12 != 0)
        throw InvalidBlobSize();

    stringstream ss(blob);

    return fromBinary(ss,N/12);
}

bool FluenceMapBase::fromBinary(istream& is,unsigned long N)
{
    unsigned i;
    for(i=0; i<N && !is.eof(); ++i)
    {
    	uint32_t id;
    	double val;
    	is.read((char*)&id, 4);
    	is.read((char*)&val,8);

    	F.insert(make_pair(id,val));
    }

    if (is.eof() && N != -1 && N != i)
    {
    	cerr << "ERROR: Ran out of bytes to read in FluenceMapBase::fromBinary" << endl;
    	return false;
    }

    cout << "Read " << i << " nonzero elements from istream in FluenceMapBase" << endl;
    return true;
}

// Writes output in TIM-OS format, except that it excludes faces with no fluence
template<class T>void FluenceMap<T>::writeTIMOS(const TetraMesh& mesh,string fn)
{
    ofstream os(fn.c_str());

    os << 1 << ' ' << F.size() << ' ' << 1 << endl;

    for(const_IDp_fluence_iterator it=fluenceByIDpBegin(); it != fluenceByIDpEnd(); ++it)
    {
        pair<T,double> p=*it;
        os << plainwhite << p.first << mesh.getFaceArea(p.first) << ' ' << p.second << endl;
    }
}

void FluenceMapBase::writeASCII(string fn)
{
    ofstream os(fn.c_str());
    for(const_iterator it=begin(); it != end(); ++it)
        os << it->first << ' ' << it->second << endl;
}

//Blob HitMap::toBinary() const
//{
//	Blob b(size()*12);	// 12= uint32 (4) + uint64 (8)
//	uint8_t* p=b.getWritePtr();
//	for(const_iterator it=begin(); it != end(); ++it)
//	{
//		*(uint32_t*)p 		= it->first;
//		*(uint64_t*)(p+4)	= it->second;
//		p+= 12;
//	}
//	return b;
//}
//
//void HitMap::fromBinary(const Blob& b)
//{
//	iterator it;
//	clear();
//	if (b.getSize() == 0)
//		cerr << "Error, zero size for hit map!" << endl;
//	else if (b.getSize() % 12 != 0)
//		cerr << "Error, size not a multiple of 12 for hit map!" << endl;
//	else
//		for(const uint8_t* p=b.getPtr(); p<b.getEndPtr(); p+=12)
//			it = insert(it,make_pair(*(const uint32_t*)p,*(const uint64_t*)(p+4)));
//}

// converts a fluence map of (ID,value) into a vector where v[ID]=value
vector<double> FluenceMapBase::toVector(unsigned N) const
{
    vector<double> v(N+1,0);

    for(const_iterator it=begin(); it != end(); ++it)
        if (it->first > 0 && it->first <= N)
            v[it->first] = it->second;
        else
            cerr << "Error in map -> vector conversion; index " << it->first << " exceeds max (" << N << ")" << endl;
    return v;
}

void FluenceMapBase::loadASCII(const string& fn,unsigned long long packets_)
{
    if (packets_ != 0)
        packets=packets_;
    ifstream is(fn.c_str());

    if(!is.good()){
        cerr << "Failed to open file " << fn << " to load fluence map" << endl;
        return;
    }
    
    unsigned ID;
    double f,sum=0;

    while(!is.eof())
    {
        is >> ID >> f;
        if (!is.fail())
        {
            F[ID] += f;
            sum += f;
        }
    }
    cout << "Info: loaded a fluence map totalling " << sum << " from " << fn << endl;

    if (packets != 0)
    {
        double k = 1/(double)packets;
        for(map<unsigned,double>::iterator it=F.begin(); it != F.end(); ++it)
            it->second *= k;
    }
}
