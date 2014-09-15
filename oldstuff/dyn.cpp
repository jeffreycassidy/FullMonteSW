#include <iostream>

using namespace std;

void say_hello()
{
    cout << "Hello from the dynamically-linked library!" << endl;
}

void print_two_numbers(int x,double d)
{
    cout << x << " " << d << endl;
}
