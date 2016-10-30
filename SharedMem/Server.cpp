/*
 * Server.cpp
 *
 *  Created on: Oct 24, 2016
 *      Author: jcassidy
 */

#include <iostream>

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/xsi_shared_memory.hpp>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>


using namespace std;

int main(int argc,char **argv)
{
//	boost::interprocess::xsi_key key("abc",1000);
//	cout << "Created key" << endl;
//
//	boost::interprocess::xsi_shared_memory shm(boost::interprocess::create_only, key, 1000, boost::interprocess::permissions());

	std::size_t sz=1024;
	int shmid;

	int key = 1234;

	int st;

	shmid = shmget(key,sz,IPC_CREAT | 0666);

	cout << "shmget returned " << shmid << endl;

	if (shmid < 0)
	{
		perror("shmget");
	}

	void *p = shmat(shmid,nullptr,0);

	char *c = static_cast<char*>(p);

	if (!p)
		cout << "Returned null" << endl;
	else if (p == (void*)-1)
		cout << "Returned -1" << endl;
	else
		cout << "p=" << p << endl;

	sprintf(c,"hello");

	sleep(60);
}
