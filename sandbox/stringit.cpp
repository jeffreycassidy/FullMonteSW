#include <string>
#include <iostream>
using namespace std;
int main(int argc,char **argv)
{
    string s("hello");

    cout << "String is \"" << s << "\"" << endl;
    cout << "Length=" << s.length() << endl;
    cout << "Iterator difference is " << s.end()-s.begin() << endl;
    cout << "begin=" << s.begin() << " end=" << s.end() << endl;
    cout << "s.end+1=" << s.end()+1 << endl;
    return 0;
}
