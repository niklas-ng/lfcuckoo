//#include "Configuration.h"
#include <string>
#include <iostream>
#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include "Platform/Primitives.h"

using namespace std;

//Interface to the data-structure we decided to benchmark
/*class TestDS {
public:
	virtual bool Search(int key)=0;
	virtual int Insert(int key, int value)=0;
	virtual int Remove(int key)=0;
};*/


///////////////////////
//GLOBALS
///////////////////////
int _gNumThreads;
extern int _gTotalRandNum;
extern int * _gRandNumAry;
int volatile * _gThreadResultAry;
//Configuration			_gConfiguration;
//TestDS* _gTestDs;

/*
void initializeTable(int table_size) 
{
		for (int iRandNum = 0; iRandNum < table_size; ++iRandNum) {
		_gTestDs->Insert(_gRandNumAry[iRandNum], _gRandNumAry[iRandNum]);
	}
	cerr << "Initialize table failed" << endl;
}

void prepareRandNum() {
	_gRandNumAry = new int[_gTotalRandNum];
	int last_number =  1;
	for (int iRandNum = 0; iRandNum < _gTotalRandNum; ++iRandNum) {
		last_number+=1;
		_gRandNumAry[iRandNum] = last_number;
	}
}
*/
void test(int argc, char **argv) 
{
	//create an array of random numbers
	//prepareRandNum();

	//initialize the table
//	int table_size = (int)((_gConfiguration.initial_count) * (_gConfiguration.load_factor));
	//initializeTable(table_size);


}