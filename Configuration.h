#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP 1

#include <stdio.h>
#include <cstdlib>
#include <string>

#define MAX_COL_KEY 21700
#define MAX_COL_OP 1000
#define MAX_ROW 48
#define TT unsigned int

#ifdef _MSC_VER
#include <windows.h>
unsigned long randseed() {
  LARGE_INTEGER time_t;
  QueryPerformanceCounter(&time_t);
  return time_t.LowPart;

}
#else
#include <sys/time.h>
unsigned long randseed() {
  struct timeval starttime;
  gettimeofday(&starttime,0);
  return starttime.tv_sec*1000000+starttime.tv_usec;
}
#endif

enum Contention_Level_Type {
	CONT_HIGH = 2,
	CONT_MED = 1,
	CONT_LOW = 0
};

class Params {
public:
	void* object;
	int **rand_key;
	int **rand_int;
	unsigned int num_threads;
	int contention_level;
	int backoff;

	float	load_factor;
	int   initial_count;
	int expTime;
	char* algoName;
	int n_mod_threads;
	char *testType;
	int nUpdateOps;

	void read_distributionfile(const char *filename, int** dist_array, int max_col) {
		FILE *pfile;
		pfile = fopen(filename, "r");
		if (pfile==NULL) {
			return ;
		}
		int num = 0, res=0;
		int row=0, col=0;
		while (fscanf(pfile, "%d" , &num) != EOF) {
			dist_array[row][col++]=num;
			if (col>=max_col) {
				row++; col=0;
				if (row>=MAX_ROW) break;
			}
		}
	
		fclose(pfile);
	}

	void init(int argc, char *argv[]) {
		int argi = 1;
		algoName = argv[argi++];
		num_threads = atoi(argv[argi++]);
		expTime = atoi(argv[argi++]);
		int startwith = (unsigned int)(std::atoi(argv[argi++]));
		for (initial_count = 1;	initial_count < startwith; initial_count *= 2) {}
		int i_load_factor = atoi(argv[argi++]);
		load_factor = i_load_factor/100.0f; 
	    
		testType = argv[argi++];
		if (strcmp(testType, "threads")==0) {
			if (argc>argi)
				n_mod_threads = atoi(argv[argi++]);
			else n_mod_threads = 1;
		}else {
			nUpdateOps = atoi(argv[argi++]);
			n_mod_threads = -1;
		}
		
		rand_int = new int*[MAX_ROW];
		for (int i=0; i< MAX_ROW; i++)
				rand_int[i] = new int[MAX_COL_OP];

		rand_key = new int*[MAX_ROW];
		for (int i=0; i< MAX_ROW; i++)
				rand_key[i] = new int[MAX_COL_KEY];

		read_distributionfile("testdata.txt", rand_int, MAX_COL_OP);
		read_distributionfile("keydata.txt", rand_key, MAX_COL_KEY);

#ifdef BACKOFF
		backoff = 1;
#else 
		backoff=0;
#endif
	}

	void deallocate() {
		for (int i=0; i< MAX_ROW; i++)
			delete [] rand_int[i];
		for (int i=0; i< MAX_ROW; i++)
			delete [] rand_key[i];
		delete [] rand_int;
		delete [] rand_key;
	}
};

class Configuration {
public:
	char*	algName;
	int   test_no;
	int   no_of_threads;

	int   update_ops;
	float	load_factor;

	int   initial_count;
	int   throughputTime;
	int   is_print_result;

	bool read() {
		try {
			//read configuration from input stream
			int i_load_factor=0;
			int num_read = fscanf(stdin, "%s %d %d %d %d %d %d %d", algName, &test_no, &no_of_threads, &update_ops, &i_load_factor, &initial_count, &throughputTime, &is_print_result);
			int startwith = initial_count;
			for (initial_count = 1;	initial_count < startwith; initial_count *= 2) {}
			load_factor = i_load_factor/100.0f;
			return (14 == num_read);
		} catch (...) {
			return false;
		}
	}

	bool read(int argc, char **argv) {
		try {
			//read configuration from input stream
			int i_load_factor=0;

			int curr_arg=1;
			algName = argv[curr_arg++];
			test_no				= std::atoi(argv[curr_arg++]);
			no_of_threads		= std::atoi(argv[curr_arg++]);
			update_ops			= std::atoi(argv[curr_arg++]);
			i_load_factor		= std::atoi(argv[curr_arg++]);
			int startwith = (unsigned int)(std::atoi(argv[curr_arg++]));
			for (initial_count = 1;	initial_count < startwith; initial_count *= 2) {}
			throughputTime		= std::atoi(argv[curr_arg++]);
			is_print_result	= std::atoi(argv[curr_arg++]);
			load_factor = i_load_factor/100.0f;
			return true;
		} catch (...) {
			return false;
		}
	}

	std::string GetAlgName() {return std::string(algName);}
};

#endif
