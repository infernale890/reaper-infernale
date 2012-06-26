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

static const uint32_t KERNEL_INPUT_SIZE = 128;
static const uint32_t KERNEL_OUTPUT_SIZE = 256;

static const uint32_t WORK_EXPIRE_TIME_SEC = 120;
static const uint32_t SHARE_THREAD_RESTART_THRESHOLD_SEC = 20;

static const uint32_t TARGET_RUNTIME_MS = 320;
static const uint32_t TARGET_RUNTIME_ALLOWANCE_MS = 25;
static const uint32_t RUNTIMES_SIZE = 16;

static const uint32_t CPU_BATCH_SIZE = 1024;

static const uint32_t K[64] = 
{ 
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};


#define foreachgpu() for(std::vector<_clState>::iterator it = GPUstates.begin(); it != GPUstates.end(); ++it)
#define foreachcpu() for(std::vector<Reap_CPU_param>::iterator it = CPUstates.begin(); it != CPUstates.end(); ++it)

#if defined(_M_X64) || defined(__x86_64__)
#define REAPER_PLATFORM "64-bit"
#else
#define REAPER_PLATFORM "32-bit"
#endif

#endif
