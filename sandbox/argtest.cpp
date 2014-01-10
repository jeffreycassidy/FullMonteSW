#include <iostream>
#include <iomanip>

using namespace std;

int main(int argc,char **argv)
{
    for(unsigned i=0;i<argc;++i)
        cout << setw(2) << i << " " << argv[i] << endl;
    return 0;
}
