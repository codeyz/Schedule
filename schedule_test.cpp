#include <iostream>
#include <unistd.h>
#include "schedule.h"
using namespace std;

void func(void* arg)
{
	char* str = static_cast<char*>(arg);
	cout << "func: " << str << endl;
}

int main()
{
	Schedule* schedule = new Schedule();
	char var[] = "test";
	schedule->Sched(func, (void*)var, 6000);
	sleep(1);
	char var1[]= "test2";
	schedule->Sched(func, (void*)var1, 2000);
	schedule->Pause();
	sleep(10);
	schedule->Continue();
	sleep(5);
	schedule->Stop();
	delete schedule;
	return 0;
}
