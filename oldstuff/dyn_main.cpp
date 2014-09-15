#include <iostream>
#include <dlfcn.h>

using namespace std;

int main(int argc,char **argv)
{
    void *lib_handle;
    char *error;

    void (*fn)();

    void (*print_two)(int,double);
    
    lib_handle = dlopen("libdyn.so",RTLD_NOW);
    if(!lib_handle){
        cerr << "Failed to open library" << endl;
        return -1;
    }

    fn = (void(*)())dlsym(lib_handle,"_Z9say_hellov");
    if ((error=dlerror()) != NULL)
    {
        cerr << "Failed to resolve symbols" << endl;
        return -2;
    }

    print_two = (void(*)(int,double))dlsym(lib_handle,"_Z17print_two_numbersid");
    if ((error=dlerror()) != NULL)
    {
        cerr << "Failed to find print_two" << endl;
        return -3;
    }

    (*fn)();
    (*print_two)(1,2.0);

    dlclose(lib_handle);
    return 0;
}
