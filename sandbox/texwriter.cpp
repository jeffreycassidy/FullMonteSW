#include <boost/tuple/tuple.hpp>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <cmath>

using namespace std;

typedef vector<string>::const_iterator LabelIterator;

template<class T>void printColTypes(ostream& os,const T& t,bool vlines=true)
    { if(vlines) os << '|'; os << t.get_head().getColType(); printColTypes(os,t.get_tail()); }
template<>void printColTypes<boost::tuples::null_type>(ostream& os,const boost::tuples::null_type&,bool vlines)
    { if(vlines) os << '|'; }

template<class T>void printHeaders(ostream& os,const boost::tuples::cons<T,boost::tuples::null_type>& t,bool vlines=true)
    { os << t.get_head().getLabel(vlines); }
template<class T>void printHeaders(ostream& os,const T& t,bool vlines=true)
    { os << t.get_head().getLabel(vlines) << " & "; printHeaders(os,t.get_tail()); }

//template<class T>void printValues(ostream& os,const boost::tuples::cons<T,boost::tuples::null_type>& t,const boost::tuples::cons<typename T::type,boost::tuples::null_type>& v)
//    { os << " & " << t.get_head()(v.get_head()); }

//void printValues(ostream&,const boost::tuples::null_type&,const boost::tuples::null_type&){}

template<class T,class V>void printValues(ostream& os,const T& t,const V& v)
    { os << t.get_head()(v.get_head()); os << " & "; printValues(os,t.get_tail(),v.get_tail()); }

template<class T,class V>void printValues(ostream& os,const boost::tuples::cons<T,boost::tuples::null_type>& t,const boost::tuples::cons<V,boost::tuples::null_type>& v)
    { os << t.get_head()(v.get_head()); }



template<class T,class V>class TeXTable {
    string label,caption;

    bool hlines,vlines;

    T printers;
    ostream& os;

    public:
    TeXTable(const string& label_,const string& caption_,const T& t_,ostream& os_) :
        label(label_),caption(caption_),hlines(true),vlines(true),printers(t_),os(os_){ header(); }
    ~TeXTable(){ footer(); }

    void operator()(const V& v) const {
        printValues(os,printers,v);
        os << "\\\\" << endl;
    }

    void header();
    void footer();
};

template<class T,class V>void TeXTable<T,V>::header()
{
    os << "\\begin{table}"      << endl;
    os << "\\begin{tabular}{";

    printColTypes<T>(os,printers,vlines);

    os << "}" << endl;

    if (hlines)
        os << "\\hline " << endl;

    printHeaders<T>(os,printers);

    os << "\\\\" << endl;

    if (hlines)
        os << "\\hline " << endl;
}

template<class T,class V>void TeXTable<T,V>::footer()
{
    if (hlines)
        os << "\\hline " << endl;
    os << "\\end{tabular}" << endl;
    if(label.size() > 0)
        os << "\\label{tbl:" << label << "}" << endl;
    if(caption.size() > 0)
        os << "\\caption{" << caption << "}" << endl;
    os << "\\end{table}" << endl;
}

template<class T>class TeXPrinter {
    string label;
    public:
    typedef T type;
    TeXPrinter(const TeXPrinter<T>& p) : label(p.label){};
    TeXPrinter(const string& label_=string("")) : label(label_){}
    string getColType() const           { return "l"; }
    string getLabel(bool=true) const    { return label; }

    string operator()(const T& v) const { stringstream ss; ss << v; return ss.str(); }
};

template<>class TeXPrinter<boost::tuples::null_type> {
    public:
    typedef boost::tuples::null_type type;
    string getColType() const           { return ""; }
    string getLabel(bool=true) const    { return ""; }
    string operator()(const type&) const{ return ""; }

};

template<>class TeXPrinter<double> {
    string colLabel;
    double multconst;

    public:
    typedef double type;
    TeXPrinter(const string& colLabel_,unsigned decs_=2) : colLabel(colLabel_),multconst(1.0)
        { for(unsigned i=0; i<decs_; ++i) multconst *= 10.0; }

    string operator()(const type&) const;

    string getColType() const { return "r@.l"; }
    string getLabel(bool vlines=true)  const
        { return string("\\multicolumn{2}{") + (vlines?"|c|}{":"c}{") + colLabel+"}"; }
};

string TeXPrinter<double>::operator()(const double& x) const
{
    stringstream ss;
    ss << (unsigned long long)(floor(x)) << '&' << (unsigned)(rint(fmod(multconst*x,multconst)));
    return ss.str();
}


int main(int argc,char **argv)
{
    typedef boost::tuples::tuple<string,unsigned,double,string> test_row;

    typedef boost::tuples::tuple<TeXPrinter<string>,TeXPrinter<unsigned>,TeXPrinter<double>,TeXPrinter<string> > test_row_print;

    test_row blah("hello",2,1234.5,"world");

    test_row_print p = boost::tuples::make_tuple(
        TeXPrinter<string>("Astring"),
        TeXPrinter<unsigned>("int"),
        TeXPrinter<double>("Runtime",5),
        TeXPrinter<string>("Morestring")
        );

    string a("string a"),d("string d");
    unsigned b=69;
    double c=3.14159;

    {
        TeXTable<test_row_print,test_row> t("tablelabel","table caption",p,cout);
        t(boost::tuples::tie(a,b,c,d));
    }
}
