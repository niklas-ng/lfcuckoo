enum architecture_t {
	INTEL,
	AMD_BULLDOZER,
	OTHERS
};

enum strategy_t {
	JUMP_SOCKET,
	FILL_SOCKET
};


struct platform_desc_t {
	architecture_t _arch;
	strategy_t _strat;
	int _nrCores_per_sock;
	int _nrSockets;
	int _nrPU_per_sock; //number of processing unit (after HT, 1 core with HT is counted as 2 PU
public:
	platform_desc_t(architecture_t arch, strategy_t affinity_method, int nrCores_per_sock, int nrSockets, int nrPU_per_sock) 
	{
		_nrCores_per_sock = nrCores_per_sock;
		_nrSockets = nrSockets;
		_nrPU_per_sock = nrPU_per_sock;
		_arch = arch;
		_strat = affinity_method;
	}
};

void setThreadAffinity(int ordThread, platform_desc_t *platform);
