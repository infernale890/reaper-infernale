#ifndef GLOBAL_H
#define GLOBAL_H

#include "CMakeConf.h"

#define _CRT_SECURE_NO_WARNINGS
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <cstdint>
#include <vector>
#include <iostream>
#include <string>
#include <deque>


#include "Config.h"

struct Coin
{
	std::string name;
	std::string protocol;
	Config config;
	uint32_t global_worksize;
	uint32_t local_worksize;
	uint32_t threads_per_gpu;
	bool max_aggression;
	double sharekhs;

	uint32_t cputhreads;
	std::string cpu_algorithm;
	std::string host,port,user,pass,proxy;
};

struct GlobalConfs
{
	bool save_binaries;
	std::vector<uint32_t> devices;
	uint32_t platform;
	Coin coin;
};

enum SHARETEST_VALUE
{
	ST_HNOTZERO,
	ST_MORETHANTARGET,
	ST_GOOD,
};

extern GlobalConfs globalconfs;
extern bool shutdown_now;

const uint32_t KERNEL_INPUT_SIZE = 128;
const uint32_t KERNEL_OUTPUT_SIZE = 256;

const uint32_t WORK_EXPIRE_TIME_SEC = 120;
const uint32_t SHARE_THREAD_RESTART_THRESHOLD_SEC = 20;

const uint32_t TARGET_RUNTIME_MS = 320;
const uint32_t TARGET_RUNTIME_ALLOWANCE_MS = 25;
const uint32_t RUNTIMES_SIZE = 16;

const uint32_t CPU_BATCH_SIZE = 1024;

#define foreachgpu() for(std::vector<_clState>::iterator it = GPUstates.begin(); it != GPUstates.end(); ++it)
#define foreachcpu() for(std::vector<Reap_CPU_param>::iterator it = CPUstates.begin(); it != CPUstates.end(); ++it)

#if defined(_M_X64) || defined(__x86_64__)
#define REAPER_PLATFORM "64-bit"
#else
#define REAPER_PLATFORM "32-bit"
#endif

#endif
