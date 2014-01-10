#include <string>
#include <iostream>
#include <iomanip>

using namespace std;

template<class T>class QueryField {
    string fexpr,fname;

    public:

    QueryField(const string& fexpr_,const string& fname_="") : fexpr(fexpr_),fname(fname_){}

    template<class U>friend ostream& operator<<(ostream&,const QueryField<U>&);
};

template<class T>ostream& operator<<(ostream& os,const QueryField<T>& q)
{
    if (q.fname != "")
        cout << q.fexpr << " AS '" << q.fname << '\'';
    else
        cout << q.fexpr;
}

int main(int argc,char **argv)
{
    QueryField<int> f0("caseID");
    QueryField<double> f1("aDouble");
    QueryField<double> f2("sum(x) + 1","aDoubleExpr");

    cout << f0 << endl;
    cout << f1 << endl;
    cout << f2 << endl;
}
