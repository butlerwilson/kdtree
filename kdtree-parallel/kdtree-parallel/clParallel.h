#pragma once
#include <CL/cl.h>
#include <iostream>
#include <vector>
#include <fstream>

#define CL_FAILURE	-1
#define BUFFSIZE	4096

struct runenv {
	cl_platform_id *re_platform;
	cl_uint re_device_num;
	cl_device_id *re_devices;
	cl_context re_context;
	std::vector<cl_program> re_program;
	std::vector<cl_command_queue> re_cmdqueue;
	cl_uint re_kernelindex;
	std::vector<cl_kernel> re_kernels;
};

class clParallel {
private:
	cl_int runningpid;
	cl_uint runenv_num;
	std::vector<struct runenv> runenvs;

	cl_int kdtp_clGetPlatformIDs();
	cl_int kdtp_clGetDeviceIDs();
	cl_int kdtp_clCreateContext();
	cl_int kdtp_clCreateCommandQueue();

	void kdtp_clQueryPlatformInfo(cl_platform_id *platformId);
	void kdtp_clQueryDeviceInfo(cl_device_id *deviceId);

	cl_program kdtp_clCreateProgramWithSource(std::string prog);
	cl_int kdtp_clBuildProgram(cl_program program);

	cl_int kdtp_clCreateKernel(std::string function);
	cl_int kdtp_clSetKernelArg(cl_uint index, size_t size, void *mem);

	cl_int kdtp_clShowErrors(cl_int errNum, std::string str);
public:
	clParallel() {
		runenv_num = 0;
	}

	~clParallel();
	cl_mem kdtp_clCreateBuffer(cl_mem_flags flags, size_t size, void *ptr);
	cl_int kdtp_clWriteBufferToKernel(cl_mem mem, size_t size, void *ptr);
	cl_int kdtp_clReadBufferFromKernel(cl_mem mem, size_t size, void *ptr);
	void kdtp_clEnqueueWaitForEvents(cl_uint number_events, cl_event *event_list);

	cl_int kdtp_clInitOpenCLenvs();
	void   kdtp_clShowOpenCLInfo();
	void   kdtp_clConstructProgram(std::string prog);
	cl_int kdtp_clConstructKernel(std::string function, cl_uint index, size_t size, void *mem);
	cl_int kdtp_clEnqueueRunKernel(int dim, const size_t *gwsize, const size_t *lwsize, 
								   cl_uint events_num, cl_event *event_list, cl_event *event);
	cl_int kdtp_clFinishKernel();
};