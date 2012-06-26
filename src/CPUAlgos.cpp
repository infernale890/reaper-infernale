#include "../headers/Global.h"
#include "../headers/CPUMiner.h"
#include "../headers/CPUAlgos.h"
#include "../headers/Util.h"
#include "../headers/RSHash.h"

extern uint64_t cpu_shares_hwinvalid;
extern uint64_t cpu_shares_hwvalid;

extern Work current_work;
extern pthread_mutex_t current_work_mutex;

void CPU_Got_share(Reap_CPU_param* state, uint8_t* tempdata, std::vector<uint8_t>& target, uint32_t serverid);
bool CPU_Hash_Below_Target(uint8_t* hash, uint8_t* target);

void* Reap_CPU_V3(void* param)
{
	Reap_CPU_param* state = (Reap_CPU_param*)param;

	Work tempwork;
	tempwork.time = 13371337;

	uint8_t tempdata[1536];
	memset(tempdata, 0, 1536);

	uint8_t finalhash[32];
	uint8_t hash_results[3] = {};

	uint32_t current_server_id;

	while(!shutdown_now)
	{
		if (current_work.old)
		{
			Wait_ms(20);
			continue;
		}
		if (tempwork.time != current_work.time)
		{
			pthread_mutex_lock(&current_work_mutex);
			tempwork = current_work;
			pthread_mutex_unlock(&current_work_mutex);
			memcpy(tempdata, &tempwork.data[0], 128);
			memcpy(tempdata+512, &tempwork.data[0], 128);
			memcpy(tempdata+1024, &tempwork.data[0], 128);
			*(uint32_t*)&tempdata[100] = state->thread_id;
			*(uint32_t*)&tempdata[612] = state->thread_id+0x100000;
			*(uint32_t*)&tempdata[1124] = state->thread_id+0x200000;
			current_server_id = tempwork.server_id;
		}

		*(uint64_t*)&tempdata[76] = tempwork.ntime_at_getwork + (ticker()-tempwork.time)/1000;
		*(uint64_t*)&tempdata[76+512] = *(uint64_t*)&tempdata[76];
		*(uint64_t*)&tempdata[76+1024] = *(uint64_t*)&tempdata[76];

		for(uint32_t h=0; h<CPU_BATCH_SIZE; ++h)
		{
			BlockHash_1_mine_V3(tempdata, finalhash, hash_results);
			if (hash_results[0])
			{
				BlockHash_1(tempdata, finalhash);
				if (finalhash[30] != 0 || finalhash[31] != 0)
					cpu_shares_hwinvalid++;
				else
					cpu_shares_hwvalid++;
				if (CPU_Hash_Below_Target(finalhash, &tempwork.target_share[0]))
					CPU_Got_share(state,tempdata,tempwork.target_share,current_server_id);
			}
			if (hash_results[1])
			{
				BlockHash_1(tempdata+512, finalhash);
				if (finalhash[30] != 0 || finalhash[31] != 0)
					cpu_shares_hwinvalid++;
				else
					cpu_shares_hwvalid++;
				if (CPU_Hash_Below_Target(finalhash, &tempwork.target_share[0]))
					CPU_Got_share(state,tempdata+512,tempwork.target_share,current_server_id);
			}
			if (hash_results[2])
			{
				BlockHash_1(tempdata+1024, finalhash);
				if (finalhash[30] != 0 || finalhash[31] != 0)
					cpu_shares_hwinvalid++;
				else
					cpu_shares_hwvalid++;
				if (CPU_Hash_Below_Target(finalhash, &tempwork.target_share[0]))
					CPU_Got_share(state,tempdata+1024,tempwork.target_share,current_server_id);
			}
			++*(uint32_t*)&tempdata[108];
			++*(uint32_t*)&tempdata[108+512];
			++*(uint32_t*)&tempdata[108+1024];
		}
		state->hashes += CPU_BATCH_SIZE*3;
	}
	pthread_exit(NULL);
	return NULL;
}

void* Reap_CPU_V2(void* param)
{
	Reap_CPU_param* state = (Reap_CPU_param*)param;

	Work tempwork;
	tempwork.time = 13371337;

	uint8_t tempdata[1024];
	memset(tempdata, 0, 1024);

	uint8_t finalhash[32];
	uint8_t hash_results[2] = {};

	uint32_t current_server_id;

	while(!shutdown_now)
	{
		if (current_work.old)
		{
			Wait_ms(20);
			continue;
		}
		if (tempwork.time != current_work.time)
		{
			pthread_mutex_lock(&current_work_mutex);
			tempwork = current_work;
			pthread_mutex_unlock(&current_work_mutex);
			memcpy(tempdata, &tempwork.data[0], 128);
			memcpy(tempdata+512, &tempwork.data[0], 128);
			*(uint32_t*)&tempdata[100] = state->thread_id;
			*(uint32_t*)&tempdata[612] = state->thread_id+0x100000;
			current_server_id = tempwork.server_id;
		}

		*(uint64_t*)&tempdata[76] = tempwork.ntime_at_getwork + (ticker()-tempwork.time)/1000;
		*(uint64_t*)&tempdata[76+512] = tempwork.ntime_at_getwork + (ticker()-tempwork.time)/1000;

		for(uint32_t h=0; h<CPU_BATCH_SIZE; ++h)
		{
			BlockHash_1_mine_V2(tempdata, finalhash, hash_results);
			if (hash_results[0])
			{
				BlockHash_1(tempdata, finalhash);
				if (finalhash[30] != 0 || finalhash[31] != 0)
					cpu_shares_hwinvalid++;
				else
					cpu_shares_hwvalid++;
				if (CPU_Hash_Below_Target(finalhash, &tempwork.target_share[0]))
					CPU_Got_share(state,tempdata,tempwork.target_share,current_server_id);
			}
			if (hash_results[1])
			{
				BlockHash_1(tempdata+512, finalhash);
				if (finalhash[30] != 0 || finalhash[31] != 0)
					cpu_shares_hwinvalid++;
				else
					cpu_shares_hwvalid++;
				if (CPU_Hash_Below_Target(finalhash, &tempwork.target_share[0]))
					CPU_Got_share(state,tempdata+512,tempwork.target_share,current_server_id);
			}
			++*(uint32_t*)&tempdata[108];
			++*(uint32_t*)&tempdata[108+512];
		}
		state->hashes += CPU_BATCH_SIZE*2;
	}
	pthread_exit(NULL);
	return NULL;
}

void* Reap_CPU_V1(void* param)
{
	Reap_CPU_param* state = (Reap_CPU_param*)param;

	Work tempwork;
	tempwork.time = 13371337;

	uint8_t tempdata[512];
	memset(tempdata, 0, 512);

	uint8_t finalhash[32];
	uint8_t hash_results[1] = {};

	uint32_t current_server_id;

	while(!shutdown_now)
	{
		if (current_work.old)
		{
			Wait_ms(20);
			continue;
		}
		if (tempwork.time != current_work.time)
		{
			pthread_mutex_lock(&current_work_mutex);
			tempwork = current_work;
			pthread_mutex_unlock(&current_work_mutex);
			memcpy(tempdata, &tempwork.data[0], 128);
			*(uint32_t*)&tempdata[100] = state->thread_id;
			current_server_id = tempwork.server_id;
		}

		*(uint64_t*)&tempdata[76] = tempwork.ntime_at_getwork + (ticker()-tempwork.time)/1000;

		for(uint32_t h=0; h<CPU_BATCH_SIZE; ++h)
		{
			BlockHash_1_mine_V1(tempdata, finalhash, hash_results);
			if (hash_results[0])
			{
				BlockHash_1(tempdata, finalhash);
				if (finalhash[30] != 0 || finalhash[31] != 0)
					cpu_shares_hwinvalid++;
				else
					cpu_shares_hwvalid++;
				if (CPU_Hash_Below_Target(finalhash, &tempwork.target_share[0]))
					CPU_Got_share(state,tempdata,tempwork.target_share,current_server_id);
			}
			++*(uint32_t*)&tempdata[108];
		}
		state->hashes += CPU_BATCH_SIZE;
	}
	pthread_exit(NULL);
	return NULL;
}
