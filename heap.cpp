// searches a heap for two things:
//  first       Number of elements with "first" greater than val
//  second      Array index of element exactly matching "val"

// Args are:
//  v           The vector holding the heap
//  val         Value being searched
//  d           The current depth in the heap (0 start)
//  i           The current index in the heap (0 start)

pair<unsigned,unsigned> heapSearch(const vector<unsigned>& v,unsigned val,unsigned i=0)
{
    if (i >= v.size())          // index is beyond heap end; no contribution
        return make_pair(0,0);
    else if (v[i] == val)   // found the element; return its index
        return make_pair(0,i);
    else if (v[i] > val)    // access time is still greater; keep searching
    {
        pair<unsigned,unsigned> pl,pr;
        pl = heapSearch(v,val,(i<<1)+1);
        pr = heapSearch(v,val,(i<<1)+2);
        return make_pair(pl.first+pr.first+1,pl.second|pr.second);
    }
    else 
        return make_pair(0,0);
}

template<class Comp>void resiftHeap(vector<unsigned>& v,unsigned i,Comp compare)
{
    unsigned self=i,parent=(i-1)>>1,child=(i<<1)+1;

    // swap it towards root so long as it's greater than the parent value
    if (i > 0 && compare(v[self],v[parent]))
    {
        while (self > 0 && compare(v[self],v[parent]))
        {
            swap(v[self],v[parent]);
            self = parent;
            parent = (self-1)>>1;
        }
    }
    else
    {
        // swap it towards the leaves so long as it's less than the child value
        while (child < v.size())
        {
            unsigned swap_el=self;
            if (compare(v[child],v[self]))
                swap_el = child;
            if (child+1 < v.size() && compare(v[child+1],v[swap_el]))
                swap_el = child+1;

            if (swap_el != self)
                swap(v[self],v[swap_el]);
            else
                break;

            self = swap_el;
            child = (self << 1) + 1;
        }
    }
}

bool compareGT(unsigned a,unsigned b){ return a>b; }
bool compareLT(unsigned a,unsigned b){ return a<b; }

// counts the number of distinct addresses accessed since the last access to a given address
template<class AddressInputIterator>void getRecurrenceTime2(AddressInputIterator first,AddressInputIterator last,unsigned maxaddr,SparseHistogram& hist)
{
    unsigned total_start = hist.getTotal();
    unsigned seq=0,Ncold=0,addr,Naddr=0,accessCount,Naccess=0;

    vector<unsigned> addrSeq(maxaddr,-1);       // [addr] -> [last access sequence]
    vector<unsigned> seqAddr;                   // heap of [last access sequence]

    seqAddr.reserve(maxaddr);

    // manage the progress timer
    ProgressPrinter pp(cout,&seq,8);
    NewTimer<ProgressPrinter> tmr(1.0,pp,true);

    for(AddressInputIterator curr=first; curr != last; ++curr,++seq)
    {
        // check address valid
        addr = curr->ID;
        accessCount = curr->count;
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
            seqAddr.push_back(seq);
            push_heap(seqAddr.begin(),seqAddr.end(),compareLT);
//            cout << "Cold miss - addr=" << addr << " seq=" << seq << endl;
            if (accessCount > 1)
                hist.add(1,accessCount-1);
        }
        else {
//            sa_it = seqAddr.find(addrSeq[addr]);

            pair<unsigned,unsigned> p = heapSearch(seqAddr,addrSeq[addr]);
/*
            cout << "Heap contents before: ";
            for (auto it = seqAddr.begin(); it != seqAddr.end(); ++it)
                cout << *it << ' ';
            cout << endl;*/

//            cout << "heapSearch found access to " << addr << " at seq# " << addrSeq[addr] << " with " << p.first << " successors" << endl;

            unsigned Ndistinct = p.first;

            seqAddr[p.second] = seq;
//            make_heap(seqAddr.begin(),seqAddr.end(),compareLT);

            resiftHeap(seqAddr,p.second,compareGT);

/*            cout << "Heap contents after: ";
            for (auto it = seqAddr.begin(); it != seqAddr.end(); ++it)
                cout << *it << ' ';
            cout << endl;*/
//            if(!is_heap(seqAddr.begin(),seqAddr.end(),compareLT))
//                cout << "Heap-property hat die arschkarte geziehen" << endl;

            hist.add(Ndistinct);
            if(accessCount > 1)
                hist.add(1,accessCount-1);

            addrSeq[addr] = seq;

//            cout << "Recurrence - addr=" << addr << " seq=" << seq << " after " << Ndistinct << " distinct addresses" << endl;
        }
    }

    cout << "Address | Last access time" << endl;

    for(unsigned i=0; i<maxaddr; ++i)
    {
        if (addrSeq[i] != -1)
            cout << setw(7) << i << "   " << addrSeq[i] << endl;
    }

/*    cout << "Heap contents" << endl;
    for (auto it = seqAddr.begin(); it != seqAddr.end(); ++it)
        cout << *it << ' ';*/

    unsigned Ndistinct = count_if(addrSeq.begin(),addrSeq.end(),not_equal_predicate<int>(-1));

    cout << "Total entries in input stream:     " << seq << endl;
    cout << "Total accesses in input stream:    " << Naccess << endl;
    cout << "Cold misses:                       " << Ncold << endl;
    cout << "Total hits (infinite cache size):  " << hist.getTotal()-total_start << endl;
    cout << "Total distinct addresses:          " << Ndistinct << endl;

    cout << "  checksum (cold + total hits - addresses): " << hist.getTotal()-total_start+Ncold-Naccess << endl;
    cout << "  checksum (cold - total distinct):         " << Ncold-Ndistinct << endl;

}
