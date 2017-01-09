#pragma once

#ifdef _MSC_VER

#include <windows.h>

class SimpleTimer
{
	LARGE_INTEGER starttime;
public:
	void start();
	double end();
};

static void mysleep(const long millis) {
	Sleep(millis);
}

#else
#include <time.h>

class SimpleTimer
{
	struct timeval starttime;
public:
	void start();
	double end();
};

static void mysleep(const long millis) {
	timespec delta;
	const long num_sec = millis/1000;
	delta.tv_nsec = (millis - num_sec*1000) * 1000000;
	delta.tv_sec = num_sec;
	nanosleep(&delta, 0);
}

#endif
