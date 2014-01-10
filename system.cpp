class SystemPipe {
    int fd[2];
    public:
    class ExcPipeFail {
        public:
        int code;
        ExcPipeFail(int code_) : code(code_){
            cerr << "Pipe creation failed with error message: " << strerror(code) << endl;
        }
    };

    SystemPipe()
    {
        int st;
        if ((st=pipe(fd)) != 0) throw ExcPipeFail(st);
    }

    ~SystemPipe(){
        closeRead();
        closeWrite();
    }

    void closeRead() { if (fd[0] != -1) close(fd[0]); fd[0]=-1; }
    void closeWrite(){ if (fd[1] != -1) close(fd[1]); fd[1]=-1; }

    int getReadHandle() const  { return fd[0]; }
    int getWriteHandle() const { return fd[1]; }
};

/*    errPipe.closeWrite();
    outPipe.closeWrite();

    int Nb;
    char buf[101];

    fd_set fds_read;

    boost::iostreams::file_descriptor_source outdevice(outPipe.getReadHandle(),boost::iostreams::close_handle);
    boost::iostreams::stream<boost::iostreams::file_descriptor_source> outstream(outdevice);

    boost::iostreams::file_descriptor_source errdevice(errPipe.getReadHandle(),boost::iostreams::close_handle);
    boost::iostreams::stream<boost::iostreams::file_descriptor_source> errstream(errdevice);

    int nfds=max(errPipe.getReadHandle(),outPipe.getReadHandle());
    bool out_live=true,err_live=true;

    while(err_live || out_live)
    {
        FD_ZERO(&fds_read);
        if(out_live)
            FD_SET(outPipe.getReadHandle(),&fds_read);
        if(err_live)
            FD_SET(errPipe.getReadHandle(),&fds_read);

        if (select(nfds+1,&fds_read,NULL,NULL,NULL) < 0)
        {
            cerr << "Error in select()" << endl;
            close(outPipe.getReadHandle());
            close(errPipe.getReadHandle());
            return -1;
        }

        if (FD_ISSET(outPipe.getReadHandle(),&fds_read))
        {
            string str;
            getline(outstream,str);
//            cout << "LOG: " << str << endl;

            if (outstream.eof())
                out_live = false;
        }

        if (FD_ISSET(errPipe.getReadHandle(),&fds_read))
        {
            string str;
            getline(errstream,str);
//            cout << "ERR: " << str << endl;

            if (errstream.eof())
                err_live = false;
        }
    }*/
