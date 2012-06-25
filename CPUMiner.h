#ifndef CPUMINER_H
#define CPUMINER_H

#include "pthread.h"

struct Reap_CPU_param
{
	uint32_t thread_id;

	pthread_t thread;

	bool shares_available;
	std::deque<Share> shares;
	pthread_mutex_t share_mutex;

	uint64_t hashes;
};

class CPUMiner
{
private:
public:
	void Init();
	void Quit();
};

#endif
