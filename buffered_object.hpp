template<class T>BufferedObject {
    unsigned bufSize;
    T* const first;
    T* const last;
    T* current;

    // this method is called when the buffer has reached the end
    virtual void atEnd(T*,unsigned N)=0;

    public:

    // parameter init specifies whether atEnd should be called before use or after full
    BufferedObject(unsigned bufSize_,bool init_) : bufSize(bufSize_),first(new T[bufSize_]),last(first+bufSize),
        current(init_ ? last : first){}

    ~BufferedObject(){ delete[] first; }

    T* next()
    {
        if(current==last)
        {
            atEnd(first,N);
            current=first;
        }
        return current++;
    }
};
