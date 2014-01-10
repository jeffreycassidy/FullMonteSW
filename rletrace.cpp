#include <set>
#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <inttypes.h>
#include <sys/stat.h>

#include <boost/iterator/transform_iterator.hpp>

#include "progress.hpp"

using namespace std;

template<class T>class ExtractionStream {
    istream& is;
    int pos;
    T last_item;

    public:

    ExtractionStream(istream& is_,bool end_of_stream_=false) : is(is_),pos(end_of_stream_ ? -1 : 0)
        { if(!end_of_stream_) is >> last_item; }
    ExtractionStream(const ExtractionStream& es_) : is(es_.is),pos(es_.pos),last_item(es_.last_item){}

    T operator*() const { return last_item; }
    const T* operator->() const { return &last_item; }

    bool operator==(const ExtractionStream& es) const { return es.is==is && es.pos == pos; }
    bool operator!=(const ExtractionStream& es) const { return !operator==(es); }
    ExtractionStream& operator++(){ is >> last_item; ++pos; if (is.eof()) pos=-1; return *this; }
};

struct always_true_predicate {
    template<class T>bool operator()(const T& t) const { return true; }
};

template<class T>struct not_equal_predicate {
    T x;
    not_equal_predicate(T x_) : x(x_){}
    bool operator()(const T& t) const { return t != x; }
};

typedef struct {
    uint32_t ID;
    uint32_t count;
} RLERec;

istream& operator>>(istream& is,RLERec& r)
{
    return is >> r.ID >> r.count;
}

const unsigned& getAddress(const RLERec& r){ return r.ID; }

class SparseHistogram : private map<unsigned,unsigned> {
    unsigned N;
    public:

    SparseHistogram() : N(0){}
    SparseHistogram(SparseHistogram&& a) : map<unsigned,unsigned>(std::move<map<unsigned,unsigned> >(a)),N(a.N){}

    void operator()(pair<unsigned,unsigned> p){ add(p.first,p.second); }
    void operator()(unsigned b,unsigned N_=1){ add(b,N_); }

    void add(unsigned b,unsigned N_=1){ map<unsigned,unsigned>::operator[](b) += N_; N += N_; }
    void add(pair<unsigned,unsigned> p){ add(p.first,p.second); }

    // InputIterator must dereference to either an unsigned or a pair<unsigned,unsigned>
    template<class InputIterator>void add(InputIterator first,InputIterator last);

    unsigned getTotal() const { return N; }

    void print() const;

    void writeText(string) const;
};

template<class InputIterator>void SparseHistogram::add(InputIterator first,InputIterator last)
{
    for(; first != last; ++first)
        add(*first);
}

struct PairWriter {
    ostream& os;
    PairWriter(ostream& os_) : os(os_){}
    template<class T,class U>void operator()(const pair<T,U>& p) const { os << p.first << ' ' << p.second << endl; }
};

void SparseHistogram::writeText(string fn) const
{
    ofstream os(fn);
    for_each(begin(),end(),PairWriter(os));
}

template<class AddressInputIterator>void getRecurrenceTime(AddressInputIterator first,AddressInputIterator last,
    unsigned maxaddr,SparseHistogram& h,unsigned size_hint=0);

template<class AddressInputIterator>void getDwellTime(AddressInputIterator first,AddressInputIterator last,SparseHistogram& h);

// WORKING: Temporal & Spatial caches
// 

// Static cache holds a fixed set of addresses defined by cacheSet
class StaticCache {
    const set<unsigned>& cacheSet;
    unsigned accesses,hits,misses;

    public:

    StaticCache(const set<unsigned>& s_) : cacheSet(s_),accesses(0),hits(0),misses(0){}

    unsigned getAccesses() const { return accesses; }
    unsigned getMisses()   const { return misses;   }
    unsigned getHits()     const { return hits;     }

    bool operator()(unsigned,unsigned=1);
    bool operator()(RLERec r){ return operator()(r.ID,r.count); }

    friend ostream& operator<<(ostream&,const StaticCache&);
};

bool StaticCache::operator()(unsigned addr,unsigned count)
{
    bool hit=cacheSet.count(addr)==1;
    accesses += count;
    if (hit)
        hits += count;
    else
        misses += count;
    return hit;
}

ostream& operator<<(ostream& os,const StaticCache& sc)
{
    os << "Static cache of size " << sc.cacheSet.size() << endl;
    os << "  Hits: " << sc.hits << endl;
    os << "  Misses: " << sc.misses << endl;
    os << "  Accesses: " << sc.accesses << endl;
    os << "  Miss rate: " << double(sc.misses)/double(sc.accesses)*100.0 << "%" << endl;
    return os;
}

// Temporal cache holds the N most recently accessed elements
class TemporalCache {
    typedef struct {
        unsigned addr,last_access;
    } CacheEntry;
    vector<CacheEntry> cache;
    unsigned N,accesses,hits,misses;
    public:

    TemporalCache(unsigned N_) : N(N_),accesses(0),hits(0),misses(0){ cache.reserve(N); }

    unsigned getAccesses() const { return accesses; }
    unsigned getMisses()   const { return misses;   }
    unsigned getHits()     const { return hits;     }

    bool operator()(unsigned addr,unsigned count=1);
    bool operator()(RLERec r){ return operator()(r.ID,r.count); }

    friend ostream& operator<<(ostream&,const TemporalCache&);
};

ostream& operator<<(ostream& os,const TemporalCache& tc)
{
    os << "Fully-associative LRU temporal cache of size " << tc.N << endl;
    os << "  Hits: " << tc.hits << endl;
    os << "  Misses: " << tc.misses << endl;
    os << "  Accesses: " << tc.accesses << endl;
    os << "  Miss rate: " << double(tc.misses)/double(tc.accesses)*100.0 << "%" << endl;
    return os;
}

bool TemporalCache::operator()(unsigned addr,unsigned count)
{
    unsigned i_lru=0,t_lru=-1;
    bool hit=false;
    accesses += count;

    for(unsigned i=0;i<cache.size();++i)
    {
        hit = cache[i].addr==addr;
        if (hit)
        {
            hits += count;
            cache[i].last_access = accesses;
            return true;
        }
        else if (cache[i].last_access < t_lru){      // find least-recently accessed
            t_lru = cache[i].last_access;
            i_lru = i;
        }
    }

    // we get here only on a miss (hit causes return)
    CacheEntry new_ce = { addr,accesses };
    ++misses;
    hits += count-1;
    if (cache.size() != N)
        cache.push_back(new_ce);
    else
        cache[i_lru] = new_ce;
    return false;
}


class TraceFile {
    string fn;
    ifstream is;
    RLERec current;
    unsigned N;
    unsigned curr_i;

    public:

    // const_iterator should model Input Iterator
    class iterator {
        TraceFile& tf;
        unsigned i;

        public:
        typedef unsigned difference_type;
        typedef unsigned value_type;
        typedef RLERec* pointer;
        typedef RLERec& reference;
        typedef input_iterator_tag iterator_category;

        iterator(TraceFile& tf_,unsigned i_=0) : tf(tf_),i(i_){}

        bool operator==(const iterator& it) const { return &it.tf == &tf && i == it.i; }
        bool operator!=(const iterator& it) const { return !operator==(it); }

        iterator& operator++() { ++i; tf.getNext(); return *this; }

        RLERec          operator*() const  { return tf.current;  }
        const RLERec*   operator->() const { return &tf.current; }
    };

    void restart() { is.clear(); is.seekg(0,ios_base::beg); curr_i=0; getNext(); }

    iterator begin() { return iterator(*this,0U); };
    iterator   end() { return iterator(*this,N); };

    TraceFile(string fn_) : fn(fn_),is(fn_.c_str(),ios_base::in | ios_base::binary),curr_i(0){
        is.seekg(0,ios_base::end);
        N = is.tellg()/sizeof(RLERec);
        is.seekg(0,ios_base::beg);
        getNext();
    }
    void getNext(){ is.read((char*)&current,sizeof(RLERec)); ++curr_i; }
};


void getHistograms(string fn, map<unsigned,unsigned>& hits,map<unsigned,unsigned>& rlhist,unsigned &N_total,unsigned &N_change,const set<unsigned>& excludeSet=set<unsigned>());

// counts the number of distinct addresses accessed since the last access to a given address
template<class AddressInputIterator>void getRecurrenceTime(AddressInputIterator first,AddressInputIterator last,unsigned maxaddr,SparseHistogram& hist,unsigned size_hint)
{
    unsigned total_start = hist.getTotal();
    unsigned seq=0,Ncold=0,addr,Naddr=0,accessCount,Naccess=0;
    vector<unsigned> addrSeq(maxaddr,-1);   // [addr] -> [last access sequence]
    map<unsigned,unsigned> seqAddr;         // [last access sequence] -> [addr]

    map<unsigned,unsigned>::iterator sa_it;

    ProgressPrinter pp(cout,&seq,8);

    NewTimer<ProgressPrinter> tmr(1.0,pp,true);

    for(AddressInputIterator curr=first; curr != last; ++curr,++seq)
    {
        // check address valid
        addr = curr->ID;
        accessCount = max(uint32_t(1),curr->count);
        Naccess += accessCount;
        if (addr>maxaddr)
        {
            cerr << "Error: address out of range" << endl;
            exit(0);
        }

        if (addrSeq[addr] == -1)       // no access yet; create new entry and record cold miss
        {
            ++Ncold;
            addrSeq[addr] = seq;
            seqAddr.insert(make_pair(seq,curr->ID));
            --accessCount;
        }

        if (accessCount > 0){
            sa_it = seqAddr.find(addrSeq[addr]);

            // count distinct addresses before and add to histogram
            unsigned Ndistinct=0;
            for(map<unsigned,unsigned>::const_iterator it = sa_it; it != seqAddr.end(); ++it,++Ndistinct){}

            hist.add(Ndistinct);
            if(accessCount > 1)
                hist.add(1,accessCount-1);

            seqAddr.erase(sa_it);
            seqAddr.insert(make_pair(seq,addr));

            addrSeq[addr] = seq;
        }
    }

    unsigned Ndistinct = count_if(addrSeq.begin(),addrSeq.end(),not_equal_predicate<int>(-1));

    cout << "Total entries in input stream:     " << seq << endl;
    cout << "Total accesses in input stream:    " << Naccess << endl;
    cout << "Cold misses:                       " << Ncold << endl;
    cout << "Total hits (infinite cache size):  " << hist.getTotal()-total_start << endl;
    cout << "Total distinct addresses:          " << Ndistinct << endl;

    cout << "  checksum (cold + total hits - addresses): " << hist.getTotal()-total_start+Ncold-Naccess << endl;
    cout << "  checksum (cold - total distinct):         " << Ncold-Ndistinct << endl;
}

// getDwellTime requires a pair<unsigned,unsigned> InputIterator
template<class AddressInputIterator>void getDwellTime(AddressInputIterator first,AddressInputIterator last,SparseHistogram& h)
{
    for(; first != last; ++first)
        h(first->count);
}


int main(int argc,char **argv)
{
    unsigned N_temporal_cache=1,N_static_cache=16384;
    unsigned N_change,N_total;
    map<unsigned,unsigned> hits,rlhist;
    if (argc < 2)
    {
        cerr << "Requires at least 1 argument; reltrace <rlefile> <L1 size> <L2 size>" << endl;
        return -1;
    }

    if (argc > 2)
        N_temporal_cache=atoi(argv[2]);

    if (argc > 3)
        N_static_cache=atoi(argv[3]);


    cout << "L1: Temporal cache, LRU size " << N_temporal_cache << endl;
    cout << "L2: Spatial cache, size " << N_static_cache << endl;

    getHistograms(argv[1],hits,rlhist,N_total,N_change);

    // create vector of hit counts and sort it
    vector<pair<unsigned,unsigned> > v; // pairs are [count, ID]
    v.resize(hits.size());

    unsigned i=0;

    for(map<unsigned,unsigned>::const_iterator it=hits.begin(); it != hits.end(); ++it,++i)
        v[i] = make_pair(it->second,it->first);

    sort(v.begin(),v.end());

    // Create cache set
    cout << "Creating cache set from top " << N_static_cache << " elements" << endl;
    set<unsigned> cacheset;
    set<unsigned> llc;
    unsigned cache_coverage=0;
    i=0;

    vector<pair<unsigned,unsigned>>::const_reverse_iterator it;

    for(it=v.rbegin(); it != v.rend() && i < N_static_cache; ++it,++i)
    {
        cache_coverage += it->first;
        cacheset.insert(it->second);
    }
    cout << "  Static cache coverage is " << double(cache_coverage)/double(N_total)*100.0 << "%" << endl;

    for(i=0; it != v.rend() && i < 4096; ++it,++i)
    {
        cache_coverage += it->first;
        llc.insert(it->second);
    }
    cout << "  Static LL cache coverage is " << double(cache_coverage)/double(N_total)*100.0 << "%" << endl;

    {
        string cache_fn("static_cache_list.txt");
        ofstream os(cache_fn.c_str(),ios_base::out);
        for(set<unsigned>::const_iterator it=cacheset.begin(); it != cacheset.end(); ++it)
            os << *it << endl;
        cout << "  Static cache set written to " << cache_fn << endl;
    }


    // Histogram of run lengths
    cout << "Run length histogram: " << endl;
    unsigned csum=0;
    for(map<unsigned,unsigned>::const_iterator it=rlhist.begin(); it != rlhist.end() && csum < 0.95*double(N_change); ++it)
    {
        csum += it->second;
        cout << setw(4) << it->first << " " << setw(10) << it->second <<
            "  (" << double(it->second)/double(N_change)*100.0 << "%, "
            << double(csum)/double(N_change)*100.0 << "% cumulative)" << endl;
    }

    {
        TraceFile tf(argv[1]);

        i=0;

        StaticCache sc(cacheset);
        StaticCache sc_ll(llc);
        TemporalCache tc(N_temporal_cache);

        for(TraceFile::iterator it = tf.begin(); it != tf.end(); ++it,++i)
        {
            RLERec tmp = *it;
            tmp.count = max(tmp.count,uint32_t(1));
            if (!tc(tmp)){
                tmp.count=1;
                if (!sc(tmp))
                    sc_ll(tmp);
            }
        }

        cout << "Level 1 (temporal)" << endl;
        cout << tc << endl;

        cout << "Level 2 (spatial)" << endl;
        cout << sc << endl;

        cout << "Level 3 (spatial)" << endl;
        cout << sc_ll << endl;

        cout << "Combined miss rate: " << double(sc_ll.getMisses())/double(tc.getAccesses())*100.0 << "%" << endl;
    }

    struct stat s;

//    SparseHistogram h = getRecurrenceTime(ExtractionStream<RLERec>(cin),ExtractionStream<RLERec>(cin,true),1000000);
    SparseHistogram h;
    char fn[100]="tetra.trace.0.bin";

    for(unsigned i=0; stat(fn,&s)==0; ++i,sprintf(fn,"tetra.trace.%d.bin",i))
    {
        TraceFile tf(fn);
        cout << fn << endl;
        getRecurrenceTime(tf.begin(),tf.end(),1000000,h,s.st_size/sizeof(RLERec));
    }

    h.writeText("recurrence.txt");



    SparseHistogram dwell;
    sprintf(fn,"tetra.trace.0.bin");

    for(unsigned i=0; stat(fn,&s)==0; ++i,sprintf(fn,"tetra.trace.%d.bin",i))
    {
        TraceFile tf(fn);
        cout << fn << endl;
        getDwellTime(tf.begin(),tf.end(),dwell);
    }
    cout << endl << "Dwell time histogram: " << endl;
    dwell.print();
    dwell.writeText("dwell.txt");
}



// getHistograms(fn,hits,rlhist,N_total,N_change,excludeSet = set<unsigned>());
//
//  fn          File name
//  hits        map<unsigned,unsigned>  Counts number of hits to each memory address
//  rlhist      mao<unsigned,unsigned>  Histogram of run lengths
//  N_total     Total memory accesses
//  N_change    Total number of records read

void getHistograms(string fn, map<unsigned,unsigned>& hits,map<unsigned,unsigned>& rlhist,unsigned& N_total,unsigned& N_change,const set<unsigned>& excludeSet)
{
    RLERec t;
    ifstream is(fn.c_str(), ios_base::in | ios_base::binary);

    is.seekg(0,ios_base::end);
    unsigned Nb = is.tellg();
    is.seekg(0,ios_base::beg);

    if(!is.good())
    {
        cerr << "Read failed" << endl;
        return;
    }

    cout << "Total entries: " << Nb/sizeof(RLERec) << endl;

    unsigned N_max=0;

    N_total=N_change=0;

    rlhist.clear();
    hits.clear();

    while (!is.eof())
    {
        is.read((char*)&t,sizeof(RLERec));
        if (!excludeSet.count(t.ID))
        {
            hits[t.ID] += t.count;
            N_total += t.count;
            rlhist[t.count]++;
            N_max = max(N_max,t.count);
            ++N_change;
        }
    }

    cout << "Total count is " << N_total << " events" << endl;
    cout << "Max is " << N_max << " events" << endl;
}


// distinct accesses since last access
// Markov-ness?

void SparseHistogram::print() const 
{
    for(const_iterator it=begin(); it != end(); ++it)
        cout << setw(6) << it->first << ": " << it->second << endl;
}


