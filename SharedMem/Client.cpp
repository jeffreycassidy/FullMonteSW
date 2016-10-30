/*
 * Client.cpp
 *
 *  Created on: Oct 24, 2016
 *      Author: jcassidy
 */

#include <sys/ipc.h>
#include <sys/shm.h>

#include <iostream>

#include <cstring>
using namespace std;


int main(int argc,char **argv)
{
	int shmid;
	int key=1234;

	std::size_t sz=1024;

	shmid = shmget(key,sz,0666);

	if (shmid < 0)
	{
		cout << "Error code returned" << endl;
		perror("shmget");
		return -1;
	}

	void* p = shmat(shmid,nullptr,0);

	const char* src = static_cast<const char*>(p);

	char dst[6];

	memcpy(dst,src,5);
	dst[5]=0;

	puts(dst);

	shmctl(shmid,IPC_RMID,nullptr);
	memcpy(dst,src,5);
}


