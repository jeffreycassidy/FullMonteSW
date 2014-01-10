#include <iostream>
#include <iomanip>

using namespace std;

struct A {
    unsigned char a;
    double b;
};

struct B {
    float a;
    unsigned char b;
};

int main(int argc,char **argv)
{
    cout << "sizeof(A)=" << sizeof(struct A) << " alignof(A)=" << __alignof__(A) << endl;
    cout << "sizeof(B)=" << sizeof(struct B) << " alignof(B)=" << __alignof__(B) << endl;
}
