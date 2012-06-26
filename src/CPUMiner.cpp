#include "../headers/Global.h"
#include "../headers/CPUMiner.h"
#include "../headers/Util.h"
#include "pthread.h"
#include "../headers/RSHash.h"

void CPU_Got_share(Reap_CPU_param* state, uint8_t* tempdata, std::vector<uint8_t>& target, uint32_t serverid)
{
	Share s(std::vector<uint8_t>(),target,serverid);
	s.data.assign(tempdata,tempdata+128);
	pthread_mutex_lock(&state->share_mutex);
	state->shares_available = true;
	state->shares.push_back(s);
	pthread_mutex_unlock(&state->share_mutex);
}

bool CPU_Hash_Below_Target(uint8_t* hash, uint8_t* target)
{
	for(int32_t i=31; i>=0; --i)
	{
		if (hash[i] > target[31-i])
			return false;
		else if (hash[i] < target[31-i])
			return true;
	}
	return true;
}

#include "../headers/CPUAlgos.h"

std::vector<Reap_CPU_param> CPUstates;

#include <map>

void CPUMiner::Init()
{
	if (globalconfs.coin.cputhreads == 0)
	{
#ifdef CPU_MINING_ONLY
		std::cout << "Config warning: cpu_mining_threads 0" << std::endl;
#endif
		return;
	}

	std::map<std::string,void*(*)(void*)> funcs;
	funcs["scalar"] = Reap_CPU_V1;
	funcs["vector2"] = Reap_CPU_V2;
	funcs["vector3"] = Reap_CPU_V3;

	std::cout << "Available CPU mining algorithms: ";
	for(std::map<std::string,void*(*)(void*)>::iterator it=funcs.begin(); it!=funcs.end(); ++it)
	{
		if (it!=funcs.begin())
			std::cout << ", ";
		std::cout << it->first;
	}
	std::cout << std::endl;

	void*(*cpualgo)(void*) = funcs[globalconfs.coin.cpu_algorithm];

	if (cpualgo == NULL)
	{
		if (globalconfs.coin.cpu_algorithm != "")
			std::cout << "CPU algorithm " << globalconfs.coin.cpu_algorithm << " not found." << std::endl;
		std::cout << "Using default: vector3" << std::endl;
		cpualgo = Reap_CPU_V3;
	}
	else
		std::cout << "Using CPU algorithm: " << globalconfs.coin.cpu_algorithm << std::endl;

	for(uint32_t i=0; i<globalconfs.coin.cputhreads; ++i)
	{
		Reap_CPU_param state;
		pthread_mutex_t initializer = PTHREAD_MUTEX_INITIALIZER;

		state.share_mutex = initializer;
		state.shares_available = false;

		state.hashes = 0;

		state.thread_id = i|0x80000000;

		CPUstates.push_back(state);
	}

	std::cout << "Creating " << CPUstates.size() << " CPU thread" << (CPUstates.size()==1?"":"s") << "." << std::endl;
	for(uint32_t i=0; i<CPUstates.size(); ++i)
	{
		std::cout << i+1 << "...";
		pthread_attr_t attr;
	    pthread_attr_init(&attr);
		int32_t schedpolicy;
		pthread_attr_getschedpolicy(&attr, &schedpolicy);
		int32_t schedmin = sched_get_priority_min(schedpolicy);
		int32_t schedmax = sched_get_priority_max(schedpolicy);
		if (i==0 && schedmin == schedmax)
		{
			std::cout << "Warning: can't set thread priority" << std::endl;
		}
		sched_param schedp;
		schedp.sched_priority = schedmin;
		pthread_attr_setschedparam(&attr, &schedp);

		pthread_create(&CPUstates[i].thread, &attr, cpualgo, (void*)&CPUstates[i]);
		pthread_attr_destroy(&attr);
	}
	std::cout << "done" << std::endl;
}

void CPUMiner::Quit()
{
}
