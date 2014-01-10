#include <set>
#include <iostream>
#include <boost/timer/timer.hpp>

/* Creates a very simple custom allocator that alloc's a fixed block of storage

For use to makes set<> operations faster, but it didn't seem to do that by much.... 

*/

using namespace std;

template<class T>class InsertOnlyAllocator {
    public:
    typedef T value_type;
    typedef T* pointer;
    typedef T& reference;
    typedef const T* const_pointer;
    typedef const T& const_reference;
    typedef unsigned long size_t;
    typedef unsigned long ptrdiff_t;
    template<class U>struct rebind {
        typedef InsertOnlyAllocator<U> other;
    };
    typedef true_type propagate_on_container_move_assignment;

    private:
    size_t i,sz;
    typedef size_t size_type;
    pointer p;

    public:
    InsertOnlyAllocator(size_t sz_=0) : i(0),sz(sz_),p(NULL){ cout << "Allocator(" << sz << ") init'd" << endl; }
    InsertOnlyAllocator(const InsertOnlyAllocator<T>& alloc) : i(alloc.i),sz(alloc.sz),p(NULL){ cout << "Allocator(" << alloc.sz << ") copied" << endl; }
    template<class U>InsertOnlyAllocator(const InsertOnlyAllocator<U>& alloc) : i(0),sz(alloc.getSize()),p(NULL){ cout << "WTF?" << endl;}

    ~InsertOnlyAllocator() { delete[] p; }

    pointer         address(reference x) const { return &x; }
    const_pointer   address(const_reference x) const { return &x; }

    pointer allocate(size_type n, allocator<void>::const_pointer hint=0){
        if (!p)
        {
            p=new T[sz];
            cout << "Allocated space for " << sz << endl;
        }
        if (i+n > sz)
            throw string("WHOA! Too many alloc requests");
        pointer tmp=p+i;
        i += n;
//        cout << "Received allocation request for " << n << endl;
        return tmp; }

    void deallocate(pointer p,size_type n){ }

    size_type max_size() const { return sz-i; }
    size_type getSize() const { return sz; }

    template<class U,class... Args>void construct(U* p, Args&&... args){ ::new((void*)p) U (forward<Args>(args)...); }
    template<class U>void destroy(U* p){ p->~U(); }
};

int main(int argc,char **argv)
{
    const unsigned N=10000000;

    {
        InsertOnlyAllocator<unsigned> alloc(N);
        set<unsigned,less<unsigned>,InsertOnlyAllocator<unsigned> > s(less<unsigned>(),alloc);
        cout << "Timing with allocator: " << endl;
        boost::timer::auto_cpu_timer t;
        for(unsigned i=0;i<N;++i)
            s.insert(rand());
    }

    {
        set<unsigned> s;
        cout << "Timing with default set: " << endl;
        boost::timer::auto_cpu_timer t;
        for(unsigned i=0;i<N;++i)
            s.insert(rand());
    }
}
