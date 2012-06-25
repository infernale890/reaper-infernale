#ifndef MTRLTINOPENCL_H
#define MTRLTINOPENCL_H

class OpenCL
{
private:
public:
	void Init();
	void Quit();

	static uint32_t GetVectorSize();
};

#include "pthread.h"

#ifndef CPU_MINING_ONLY
#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif
#endif

#include "Util.h"
#include "App.h"

#ifndef CPU_MINING_ONLY
struct _clState
{
	cl_context context;
	cl_kernel kernel;
	cl_command_queue commandQueue;
	cl_program program;
	cl_mem CLbuffer[2];
	cl_mem padbuffer8;

	uint32_t vectors;
	uint32_t thread_id;
	uint32_t offset;

	pthread_t thread;

	bool shares_available;
	std::deque<Share> shares;
	pthread_mutex_t share_mutex;

	uint64_t hashes;
};
#endif

#endif
