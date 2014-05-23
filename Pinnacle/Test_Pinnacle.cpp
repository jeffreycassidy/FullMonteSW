#include "PinnacleFile.hpp"

#include <iostream>

using namespace std;

int main(int argc,char **argv)
{
	if (argc < 2)
	{
		cerr << "Requires a file name specified at command line" << endl;
		return -1;
	}

	Pinnacle::File pf(argv[1]);

	pf.read();
}
