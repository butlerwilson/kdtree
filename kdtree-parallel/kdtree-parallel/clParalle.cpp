#include "clParallel.h"

cl_int clParallel::kdtp_clGetPlatformIDs()
{
	cl_int errNum = 0;
	cl_platform_id *platforms;

	errNum = clGetPlatformIDs(0, NULL, &runenv_num);
	if (errNum != CL_SUCCESS || runenv_num <= 0)
		kdtp_clShowErrors(errNum, "get platform number failed.");
	platforms = (cl_platform_id *)malloc(runenv_num * sizeof(cl_platform_id));
	if (platforms) {
		errNum = clGetPlatformIDs(runenv_num, platforms, NULL);
		if (errNum != CL_SUCCESS) {
			free(platforms);	platforms = NULL;
			kdtp_clShowErrors(errNum, "get platform failed.");
		}
		for (cl_uint ii = 0; ii < runenv_num; ++ii) {
			struct runenv tmp;
			tmp.re_platform = platforms++;
			runenvs.push_back(tmp);
		}
	} else
		kdtp_clShowErrors(errNum, "alloca platform memory failed.");

	return CL_SUCCESS;
}

cl_int clParallel::kdtp_clGetDeviceIDs()
{
	cl_int errNum = 0;
	cl_device_id *devices;

	for (cl_uint ii = 0; ii < runenv_num; ++ii) {
		errNum = clGetDeviceIDs(*(runenvs[ii].re_platform), CL_DEVICE_TYPE_ALL, 0, NULL, &(runenvs[ii].re_device_num));
		if (errNum != CL_SUCCESS || runenvs[ii].re_device_num <= 0)
			kdtp_clShowErrors(errNum, "can not found any devices.");
		devices = (cl_device_id *)malloc(runenvs[ii].re_device_num * sizeof(cl_device_id));
		if (devices) {
			errNum = clGetDeviceIDs(*(runenvs[ii].re_platform), CL_DEVICE_TYPE_ALL, runenvs[ii].re_device_num, devices, NULL);
			if (errNum != CL_SUCCESS)
				kdtp_clShowErrors(errNum, "get device failed.");
			runenvs[ii].re_devices = devices;
		}
	}

	return CL_SUCCESS;
}

void CL_CALLBACK contextCallback(const char *errInfo, const void *private_info,
	size_t cb, void *user_data)
{
	std::cout << "error: during create the context-" << errInfo << std::endl;
	exit(EXIT_FAILURE);
}

cl_int clParallel::kdtp_clCreateContext()
{
	cl_int errNum = 0;

	for (cl_uint ii = 0; ii < runenv_num; ++ii) {
		cl_context_properties properties[] = {
			CL_CONTEXT_PLATFORM, (cl_context_properties)(*runenvs[ii].re_platform), 0
		};
		runenvs[ii].re_context = clCreateContext(properties, runenvs[ii].re_device_num, runenvs[ii].re_devices,
			&contextCallback, NULL, &errNum);
		kdtp_clShowErrors(errNum, "create the context.");
	}

	return CL_SUCCESS;
}

cl_int clParallel::kdtp_clCreateCommandQueue()
{
	cl_int m = 0;
	cl_int errNum = 0;

	for (cl_uint ii = 0; ii < runenv_num - 1; ++ii) {
		runenvs[ii].re_kernelindex = 0;
		for (cl_uint k = 0; k < runenvs[ii].re_device_num; ++k) {
			cl_command_queue cmdQueue;
			cl_queue_properties properties[] = { 0 };
			cmdQueue = clCreateCommandQueueWithProperties(runenvs[ii].re_context, runenvs[ii].re_devices[k], properties, &errNum);
			//cmdQueue = clCreateCommandQueue(runenvs[ii].re_context, runenvs[ii].re_devices[k], 0, &errNum); //opencl 1.2
			kdtp_clShowErrors(errNum, "create the command queue.");
			runenvs[ii].re_cmdqueue.push_back(cmdQueue);
		}
	}

	return CL_SUCCESS;
}

cl_int clParallel::kdtp_clInitOpenCLenvs()
{
	kdtp_clGetPlatformIDs();
	kdtp_clGetDeviceIDs();
	kdtp_clCreateContext();
	kdtp_clCreateCommandQueue();

	return CL_SUCCESS;
}

void clParallel::kdtp_clQueryPlatformInfo(cl_platform_id *platformId)
{
	cl_int errNum = 0;
	char buffer[BUFFSIZE];

	errNum = clGetPlatformInfo(*platformId, CL_PLATFORM_PROFILE, BUFFSIZE, buffer, NULL);
	std::cout << "CL_PLATFORM_PROFILE:\t" << buffer << std::endl;
	errNum = clGetPlatformInfo(*platformId, CL_PLATFORM_VERSION, BUFFSIZE, buffer, NULL);
	std::cout << "CL_PLATFORM_VERSION:\t" << buffer << std::endl;
	errNum = clGetPlatformInfo(*platformId, CL_PLATFORM_NAME, BUFFSIZE, buffer, NULL);
	std::cout << "CL_PLATFORM_NAME:\t" << buffer << std::endl;
	errNum = clGetPlatformInfo(*platformId, CL_PLATFORM_VENDOR, BUFFSIZE, buffer, NULL);
	std::cout << "CL_PLATFORM_VENDOR:\t" << buffer << std::endl;
	errNum = clGetPlatformInfo(*platformId, CL_PLATFORM_EXTENSIONS, BUFFSIZE, buffer, NULL);
	std::cout << "CL_PLATFORM_EXTENSIONS:\t" << buffer << std::endl;
}

void clParallel::kdtp_clQueryDeviceInfo(cl_device_id *deviceId)
{
	cl_int errNum = 0;
	char buffer[BUFFSIZE] = {'\0'};
	cl_uint uint_num = 0;
	size_t uint_array[8];
	cl_device_type devtype;
	cl_ulong ulong_num = 0;

	errNum = clGetDeviceInfo(*deviceId, CL_DEVICE_NAME, BUFFSIZE, buffer, NULL);
	std::cout << "CL_DEVICE_NAME:\t" << buffer << std::endl;
	errNum = clGetDeviceInfo(*deviceId, CL_DEVICE_TYPE, sizeof(devtype), &devtype, NULL);
	std::cout << "CL_DEVICE_TYPE:\t";
	if (devtype & CL_DEVICE_TYPE_CPU) std::cout << "\tCL_DEVICE_TYPE_CPU" << std::endl;
	else if (devtype & CL_DEVICE_TYPE_GPU) std::cout << "\tCL_DEVICE_TYPE_GPU" << std::endl;
	else if (devtype & CL_DEVICE_TYPE_ACCELERATOR) std::cout << "\tCL_DEVICE_TYPE_ACCELERATOR" << std::endl;
	else if (devtype & CL_DEVICE_TYPE_ALL) std::cout << "\tCL_DEVICE_TYPE_ALL" << std::endl;

	errNum = clGetDeviceInfo(*deviceId, CL_DRIVER_VERSION, BUFFSIZE, buffer, NULL);
	std::cout << "CL_DRIVER_VERSION:\t" << buffer << std::endl;
	errNum = clGetDeviceInfo(*deviceId, CL_DEVICE_VERSION, BUFFSIZE, buffer, NULL);
	std::cout << "CL_DEVICE_VERSION:\t" << buffer << std::endl;
	errNum = clGetDeviceInfo(*deviceId, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(uint_num), &uint_num, NULL);
	std::cout << "CL_DEVICE_MAX_COMPUTE_UNITS:\t" << uint_num << std::endl;
	errNum = clGetDeviceInfo(*deviceId, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(uint_num), &uint_num, NULL);
	std::cout << "CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS:\t" << uint_num << std::endl;
	errNum = clGetDeviceInfo(*deviceId, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(uint_num) * 3, uint_array, NULL);
	std::cout << "CL_DEVICE_MAX_WORK_ITEM_SIZES:\t" << uint_array[0] << " " << uint_array[1] << " " << uint_array[2] << std::endl;
	errNum = clGetDeviceInfo(*deviceId, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(uint_num), &uint_num, NULL);
	std::cout << "CL_DEVICE_MAX_WORK_GROUP_SIZE:\t" << uint_num << std::endl;
	errNum = clGetDeviceInfo(*deviceId, CL_DEVICE_ADDRESS_BITS, sizeof(uint_num), &uint_num, NULL);
	std::cout << "CL_DEVICE_ADDRESS_BITS:\t" << uint_num << std::endl;
	errNum = clGetDeviceInfo(*deviceId, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(ulong_num), &ulong_num, NULL);
	std::cout << "CL_DEVICE_MAX_MEM_ALLOC_SIZE:\t" << ulong_num << std::endl;
	errNum = clGetDeviceInfo(*deviceId, CL_DEVICE_EXTENSIONS, BUFFSIZE, buffer, NULL);
	std::cout << "CL_DEVICE_EXTENSIONS:\t" << buffer << std::endl;
}

void clParallel::kdtp_clShowOpenCLInfo()
{
	cl_int errNum = 0;

	for (cl_uint ii = 0; ii < runenv_num; ++ii) {
		kdtp_clQueryPlatformInfo(runenvs[ii].re_platform);
		for (cl_uint k = 0; k < runenvs[ii].re_device_num; ++k) {
			kdtp_clQueryDeviceInfo(&(runenvs[ii].re_devices[k]));
		}
	}
}

cl_program clParallel::kdtp_clCreateProgramWithSource(std::string prog)
{
	cl_int errNum = 0;
	cl_program program;
	runningpid = 0;
	//runningpid = rand() % runenv_num;

	std::ifstream srcFile(prog);
	kdtp_clShowErrors(srcFile ? CL_SUCCESS : CL_FAILURE, "open cl source file failed.");

	std::string srcProg(std::istreambuf_iterator<char>(srcFile), (std::istreambuf_iterator<char>()));
	const char *src = srcProg.c_str();
	size_t length = srcProg.length();
	program = clCreateProgramWithSource(runenvs[runningpid].re_context, 1, &src, &length, &errNum);
	runenvs[runningpid].re_program.push_back(program);
	kdtp_clShowErrors(errNum, "create program failed.");
	srcFile.close();

	return program;
}

cl_int clParallel::kdtp_clBuildProgram(cl_program program)
{
	cl_int errNum = 0;

	errNum = clBuildProgram(program, runenvs[runningpid].re_device_num, runenvs[runningpid].re_devices, NULL, NULL, NULL);
	if (errNum != CL_SUCCESS) {
		char buildLog[BUFFSIZE];
		clGetProgramBuildInfo(program, runenvs[runningpid].re_devices[0], CL_PROGRAM_BUILD_LOG, BUFFSIZE, buildLog, NULL);
		std::cerr << "Error in kernel:" << std::endl;
		std::cerr << buildLog << std::endl;
		clReleaseProgram(program);
		return CL_FAILURE;
	}

	return CL_SUCCESS;
}

void clParallel::kdtp_clConstructProgram(std::string prog)
{
	cl_program program = kdtp_clCreateProgramWithSource(prog);
	cl_int errNum = kdtp_clBuildProgram(program);
	kdtp_clShowErrors(errNum, "build program failed.");
}

cl_int clParallel::kdtp_clCreateKernel(std::string function)
{
	cl_int errNum = 0;
	cl_kernel kernel;

	kernel = clCreateKernel(runenvs[runningpid].re_program[0], function.c_str(), &errNum);
	kdtp_clShowErrors(errNum, "create kernel error.");
	runenvs[runningpid].re_kernels.push_back(kernel);
	runenvs[runningpid].re_kernelindex = runenvs[runningpid].re_kernels.size() - 1;

	return CL_SUCCESS;
}

cl_int clParallel::kdtp_clSetKernelArg(cl_uint index, size_t size, void  *mem)
{
	cl_int errNum = 0;
	cl_uint ii = runenvs[runningpid].re_kernelindex;

	errNum = clSetKernelArg(runenvs[runningpid].re_kernels[ii], index, size, mem);
	kdtp_clShowErrors(errNum, "set kernel arguments failed.");

	return CL_SUCCESS;
}

cl_int clParallel::kdtp_clConstructKernel(std::string func, cl_uint index, size_t size, void *mem)
{
	cl_int errNum = 0;

	if (index == 0)
		errNum = kdtp_clCreateKernel(func);
	errNum = kdtp_clSetKernelArg(index, size, mem);

	return errNum;
}

cl_int clParallel::kdtp_clEnqueueRunKernel(int dim, const size_t *gwsize, const size_t *lwsize,
										   cl_uint events_num, cl_event *event_list, cl_event *event)
{
	cl_uint kernel_num = runenvs[runningpid].re_kernels.size();

	for (cl_uint ii = 0; ii < kernel_num; ++ii) {
		cl_int errNum = 0;
		errNum = clEnqueueNDRangeKernel(runenvs[runningpid].re_cmdqueue[ii],
		runenvs[runningpid].re_kernels[ii], dim, NULL, gwsize, lwsize, events_num, event_list, event);
		//errNum = clEnqueueTask(runenvs[runningpid].re_cmdqueue[ii], runenvs[runningpid].re_kernels[ii], 1, NULL, NULL);
	}

	return CL_SUCCESS;
}

cl_int clParallel::kdtp_clFinishKernel()
{
	runenvs[runningpid].re_kernelindex = 0;
	runenvs[runningpid].re_kernels.clear();

	return CL_SUCCESS;
}

cl_mem clParallel::kdtp_clCreateBuffer(cl_mem_flags flags, size_t size, void *ptr)
{
	cl_int errNum = 0;

	cl_mem mem = clCreateBuffer(runenvs[runningpid].re_context, flags, size, ptr, &errNum);
	kdtp_clShowErrors(errNum, "create buffer failed.");

	return mem;
}

cl_int clParallel::kdtp_clWriteBufferToKernel(cl_mem mem, size_t size, void *ptr)
{
	cl_int errNum = 0;

	errNum = clEnqueueWriteBuffer(runenvs[runningpid].re_cmdqueue[0], mem, CL_TRUE, 0, size, ptr, 0, NULL, NULL);
	kdtp_clShowErrors(errNum, "write buffer failed.");

	return CL_SUCCESS;
}
cl_int clParallel::kdtp_clReadBufferFromKernel(cl_mem mem, size_t size, void *ptr)
{
	cl_int errNum = 0;

	errNum = clEnqueueReadBuffer(runenvs[runningpid].re_cmdqueue[0], mem, CL_TRUE, 0, size, ptr, 0, NULL, NULL);

	kdtp_clShowErrors(errNum, "read buffer failed.");

	return CL_SUCCESS;
}

void clParallel::kdtp_clEnqueueWaitForEvents(cl_uint number_events, cl_event * event_list)
{
	//clEnqueueWaitForEvents(runenvs[runningpid].re_cmdqueue[0], number_events, event_list);
	clEnqueueBarrierWithWaitList(runenvs[runningpid].re_cmdqueue[0], number_events, event_list, NULL);
}

cl_int clParallel::kdtp_clShowErrors(cl_int errNum, std::string str)
{
	if (errNum != CL_SUCCESS) {
		std::cout << str.c_str() << std::endl;
		system("pause");
		exit(EXIT_FAILURE);
	}

	return CL_SUCCESS;
}

clParallel::~clParallel()
{
	for (cl_int ii = 0; ii < runenv_num; ++ii) {
		for (cl_int k = 0; k < runenvs[ii].re_device_num; ++k)
			free(runenvs[ii].re_devices[k]);
		free(runenvs[ii].re_platform);
	}
}