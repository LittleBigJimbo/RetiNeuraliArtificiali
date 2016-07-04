#include <CL/cl.h>
#include <vector>
#include <cmath>
#include <iostream>
#include "clWrapper.hpp"

#define clwInitLib() if (!g_Is_Initialized) clwInit();

//#define FINISH

typedef float real;

enum
{
	KERNEL_SIGM=0,
	KERNEL_OUT,
	KERNEL_PDER,
	KERNEL_WCOR,
	KERNEL_APWC
};

#define NUM_KERNELS 5

static cl_device_id g_cl_Device;
static cl_context g_cl_Context;
static cl_command_queue g_cl_Command_Queue;
static cl_kernel g_cl_Kernels[NUM_KERNELS];
static bool g_Is_Initialized = false;

static std::vector<cl_mem> g_cl_Outputs;
static std::vector<cl_mem> g_cl_Weights;
static std::vector<cl_mem> g_cl_WCorrec;
static std::vector<cl_mem> g_cl_PartDer;

static const char* g_Program_Source = //Ricorda Multiply than Add functions#######################
	"typedef float real;"
	"kernel void sigm(global float* r){"
	"int i = get_global_id(0);"
	"r[i] = 1 / (1 + exp(-r[i]));}"
	"kernel void out(global float* C, global float* A, global float* B, int cA){"
	"int r = get_global_id(0);"
	"float value=0;"
	"for (int i=0; i<cA; i++)"
	"value += A[r*cA+i] * B[i];"
	"C[r] = value;}"
	"kernel void wcor(global float* C, global float* A, global float* B, int wB){"
	"int r = get_global_id(0);"
	"int c = get_global_id(1);"
	"C[r*wB+c] += A[r] * B[c];}"
	"kernel void pder(global float* C, global float* A, global float* B, global float* D, int cA, int cB){"
	"int c = get_global_id(0);"
	"float value=0;"
	"for (int i=0; i<cB; i++)"
	"value += A[i] * B[i*cA+c];"
	"C[c] = value * D[c] * (1 - D[c]);}"
	"kernel void apwc(global float* A, global float* B, int wA, float eta){"
	"int r = get_global_id(0);"
	"int c = get_global_id(1);"
	"/*printf(\"%d %f\\n\", r*wA+c, A[r*wA+c]);*/"
	"A[r*wA+c] = A[r*wA+c] - (B[r*wA+c] * eta);"
	"/*printf(\"%d %f\\n\", r*wA+c, A[r*wA+c]);*/"
	"}";

extern "C"
{
	static void clwInit()
	{
		g_Is_Initialized = true;
		//Prima fase: Query devices*/
		cl_int rtn;
		cl_uint platNum=0;
		cl_platform_id* platformIDs;
		cl_uint devNum;
		cl_device_id* deviceIDs;
		cl_program program;

		clGetPlatformIDs(0, nullptr, &platNum);
		platformIDs = new cl_platform_id[platNum];
		clGetPlatformIDs(platNum, platformIDs, nullptr);

		for (unsigned int i=0; i<platNum; i++)
		{
			devNum=0;
			clGetDeviceIDs(platformIDs[i], CL_DEVICE_TYPE_GPU, 0, nullptr,
			  &devNum);
			if (devNum == 0)
				continue;
			deviceIDs = new cl_device_id[devNum];
			clGetDeviceIDs(platformIDs[i], CL_DEVICE_TYPE_GPU, devNum,
			  deviceIDs, nullptr);
			g_cl_Device = deviceIDs[0];
			delete[] deviceIDs;
		}
		delete[] platformIDs;

		if (g_cl_Device == 0)
			exit(EXIT_FAILURE);

		g_cl_Context = clCreateContext(nullptr, 1, &g_cl_Device, nullptr, nullptr, nullptr);
		g_cl_Command_Queue = clCreateCommandQueue(g_cl_Context, g_cl_Device, 0, nullptr);

		program = clCreateProgramWithSource(g_cl_Context,
		  1, &g_Program_Source, nullptr, &rtn);

		rtn = clBuildProgram(program, 1, &g_cl_Device, nullptr,
		  nullptr, nullptr);
		if (rtn != CL_SUCCESS)
		{
			char err[1000];
			clGetProgramBuildInfo(program, g_cl_Device, CL_PROGRAM_BUILD_LOG,
			  1000, err, nullptr);
			std::cerr<<err<<std::endl;
		}
		g_cl_Kernels[KERNEL_SIGM] = clCreateKernel(program, "sigm", &rtn);
		g_cl_Kernels[KERNEL_OUT] = clCreateKernel(program, "out", &rtn);
		g_cl_Kernels[KERNEL_PDER] = clCreateKernel(program, "pder", &rtn);
		g_cl_Kernels[KERNEL_WCOR] = clCreateKernel(program, "wcor", &rtn);
		g_cl_Kernels[KERNEL_APWC] = clCreateKernel(program, "apwc", &rtn);
		clReleaseProgram(program);
	}

	void clwSetup(int p_n_layers, int* p_n_neurons, real** p_output, real** p_weights, real** p_w_corr, real** p_par_der)
	{
		clwInitLib();
		cl_mem temp;
		g_cl_Weights.push_back(temp);
		g_cl_PartDer.push_back(temp);
		g_cl_WCorrec.push_back(temp);
		cl_int err;
		temp = clCreateBuffer(g_cl_Context, CL_MEM_USE_HOST_PTR, sizeof(real)* p_n_neurons[0], p_output[0], &err);
		if (err!=CL_SUCCESS) { std::cerr<<"NO1: "<<err<<std::endl; getchar(); };
		g_cl_Outputs.push_back(temp);
		for (int i=1; i<p_n_layers; i++)
		{
			temp = clCreateBuffer(g_cl_Context, CL_MEM_USE_HOST_PTR, sizeof(real)* p_n_neurons[i], p_output[i], &err);
			if (err!=CL_SUCCESS) { std::cerr<<"NO1: "<<err<<std::endl; getchar(); };
			g_cl_Outputs.push_back(temp);
			temp = clCreateBuffer(g_cl_Context, CL_MEM_USE_HOST_PTR, sizeof(real)* p_n_neurons[i], p_par_der[i], &err);
			if (err!=CL_SUCCESS) {std::cerr<<"NO2: "<<err<<std::endl; getchar(); }
			g_cl_PartDer.push_back(temp);
			temp = clCreateBuffer(g_cl_Context, CL_MEM_USE_HOST_PTR, sizeof(real)* p_n_neurons[i-1] * (p_n_neurons[i]-1), p_weights[i], &err);
			if (err!=CL_SUCCESS) {std::cerr<<"NO3: "<<err<<std::endl; getchar(); }
			g_cl_Weights.push_back(temp);
			temp = clCreateBuffer(g_cl_Context, CL_MEM_USE_HOST_PTR, sizeof(real)* p_n_neurons[i-1] * (p_n_neurons[i]-1), p_w_corr[i], &err);
			if (err!=CL_SUCCESS) {std::cerr<<"NO4: "<<err<<std::endl; getchar(); }
			g_cl_WCorrec.push_back(temp);
		}
	}

	void clwSetInput(real* p_input, int p_size)
	{
		clwInitLib();
		cl_int res = clEnqueueWriteBuffer(g_cl_Command_Queue, g_cl_Outputs[0], false, 0, sizeof(real) * (p_size-1), p_input, 0, nullptr, nullptr);
		if (res != CL_SUCCESS) { std::cerr<<"SETINPUT "<<res<<std::endl; getchar(); }
		res = clEnqueueBarrierWithWaitList(g_cl_Command_Queue, 0, nullptr, nullptr);
		if (res != CL_SUCCESS) { std::cerr<<"SETINPUT "<<res<<std::endl; getchar(); }
	}

	static void clwClearOutOrPart(int p_n_layers, int* p_n_neurons, std::vector<cl_mem> p_buffers)
	{
		clwInitLib();
		char pattern = 0;
		cl_int res;
		for (int i=1; i<p_n_layers; i++)
		{
			res = clEnqueueFillBuffer(g_cl_Command_Queue, p_buffers[i], &pattern, 1, 0, sizeof(real) * (p_n_neurons[i]-1), 0, nullptr, nullptr);
			if (res != CL_SUCCESS) {std::cerr<<"clCrearOutOrPart :"<<res<<std::endl; getchar(); }
		}
		#ifdef FINISH
		res = clFinish(g_cl_Command_Queue);
		#else
		res = clEnqueueBarrierWithWaitList(g_cl_Command_Queue, 0, nullptr, nullptr);
		#endif
		if (res != CL_SUCCESS) {std::cerr<<"clCrearOutOrPart :"<<res<<std::endl; getchar(); }
	}

	void clwClearOutput(int p_n_layers, int* p_n_neurons)
	{
		clwClearOutOrPart(p_n_layers, p_n_neurons, g_cl_Outputs);
	}

	void clwClearPartDer(int p_n_layers, int* p_n_neurons)
	{
		clwClearOutOrPart(p_n_layers, p_n_neurons, g_cl_PartDer);
	}

	void clwClearWeightOrWCorr(int p_n_layers, int* p_n_neurons, std::vector<cl_mem> p_buffers)
	{
		clwInitLib();
		char pattern = 0;
		cl_int res;
		for (int i=1; i<p_n_layers; i++)
		{
			res = clEnqueueFillBuffer(g_cl_Command_Queue, p_buffers[i], &pattern, 1, 0, sizeof(real) * p_n_neurons[i-1] * (p_n_neurons[i]-1), 0, nullptr, nullptr);
			if (res != CL_SUCCESS) {std::cerr<<"clCrearWeìghtOrWCorr: "<<res<<std::endl; getchar(); }
		}

		#ifdef FINISH
		res = clFinish(g_cl_Command_Queue);
		#else
		res = clEnqueueBarrierWithWaitList(g_cl_Command_Queue, 0, nullptr, nullptr);
		#endif
		if (res != CL_SUCCESS) {std::cerr<<"clCrearWeìghtOrWCorr: "<<res<<std::endl; getchar(); }
	}

	void clwClearWCorr(int p_n_layers, int* p_n_neurons)
	{
		clwClearWeightOrWCorr(p_n_layers, p_n_neurons, g_cl_WCorrec);
	}

	void clwCalcOutput(int p_layer, int p_n_neurons_p, int p_n_neurons_a)
	{
		clwInitLib();
		cl_int res;
		size_t size = p_n_neurons_a-1;
		clSetKernelArg(g_cl_Kernels[KERNEL_OUT], 0, sizeof(cl_mem), &g_cl_Outputs[p_layer]);
		clSetKernelArg(g_cl_Kernels[KERNEL_OUT], 1, sizeof(cl_mem), &g_cl_Weights[p_layer]);
		clSetKernelArg(g_cl_Kernels[KERNEL_OUT], 2, sizeof(cl_mem), &g_cl_Outputs[p_layer-1]);
		clSetKernelArg(g_cl_Kernels[KERNEL_OUT], 3, sizeof(cl_int), &p_n_neurons_p);
		res = clEnqueueNDRangeKernel(g_cl_Command_Queue, g_cl_Kernels[KERNEL_OUT], 1, nullptr, &size, nullptr, 0, nullptr, nullptr);
		if (res != CL_SUCCESS) { std::cerr<<"CalcOutput :"<<res<<std::endl; getchar(); }
		#ifdef FINISH
		res = clFinish(g_cl_Command_Queue);
		#else
		res = clEnqueueBarrierWithWaitList(g_cl_Command_Queue, 0, nullptr, nullptr);
		#endif
		if (res != CL_SUCCESS) { std::cerr<<"CalcOutput :"<<res<<std::endl; getchar(); }
	}

	real* clwMapOutputOrPartDer(int p_layer, int p_n_neurons, cl_map_flags p_flags, cl_mem p_buffer)
	{
		clwInitLib();
		real* res;
		cl_int rtn;
		clFinish(g_cl_Command_Queue);
		res = (real*)clEnqueueMapBuffer(g_cl_Command_Queue, p_buffer, CL_TRUE, p_flags, 0, sizeof(real)* p_n_neurons, 0, nullptr, nullptr, &rtn);
		if (rtn != CL_SUCCESS) { std::cerr<<"MapOutputOrPartDer "<<rtn<<std::endl; getchar(); }
		return res;
	}

	real* clwMapOutput(int p_layer, int p_n_neurons, cl_map_flags p_flags)
	{
		return (real*) clwMapOutputOrPartDer(p_layer, p_n_neurons, p_flags, g_cl_Outputs[p_layer]);
	}

	real* clwMapPartDer(int p_layer, int p_n_neurons, cl_map_flags p_flags)
	{
		return (real*) clwMapOutputOrPartDer(p_layer, p_n_neurons, p_flags, g_cl_PartDer[p_layer]);
	}

	void clwUnmapOutputOrPartDer(cl_mem p_buffer)
	{
		clwInitLib();
		void* ptr;
		cl_int res = clGetMemObjectInfo(p_buffer, CL_MEM_HOST_PTR, sizeof(void*), &ptr, NULL);
		if (res != CL_SUCCESS) { std::cerr<<"UnmapOutputOrPartDer: "<<res<<std::endl; getchar(); }
		res = clEnqueueUnmapMemObject(g_cl_Command_Queue, p_buffer, ptr, 0, nullptr, nullptr);
		if (res != CL_SUCCESS) { std::cerr<<"UnmapOutputOrPartDer: "<<res<<std::endl; getchar(); }
		#ifdef FINISH
		res = clFinish(g_cl_Command_Queue);
		#else
		res = clEnqueueBarrierWithWaitList(g_cl_Command_Queue, 0, nullptr, nullptr);
		#endif
		if (res != CL_SUCCESS) { std::cerr<<"UnmapOutputOrPartDer: "<<res<<std::endl; getchar(); }
	}

	void clwUnmapOutput(int p_layer)
	{
		clwUnmapOutputOrPartDer(g_cl_Outputs[p_layer]);
	}

	void clwUnmapPartDer(int p_layer)
	{
		clwUnmapOutputOrPartDer(g_cl_PartDer[p_layer]);
	}

	void clwSigmOutput(int p_layer, int p_n_neurons)
	{
		clwInitLib();
		size_t size = p_n_neurons-1;
		clSetKernelArg(g_cl_Kernels[KERNEL_SIGM], 0, sizeof(cl_mem), &g_cl_Outputs[p_layer]);
		cl_int rtn = clEnqueueNDRangeKernel(g_cl_Command_Queue, g_cl_Kernels[KERNEL_SIGM], 1, nullptr, &size, nullptr, 0, nullptr, nullptr);
		if (rtn != CL_SUCCESS) { std::cerr<<"SigmOutput: "<<rtn<<std::endl; getchar(); }
		#ifdef FINISH
		rtn = clFinish(g_cl_Command_Queue);
		#else
		rtn = clEnqueueBarrierWithWaitList(g_cl_Command_Queue, 0, nullptr, nullptr);
		#endif
		if (rtn != CL_SUCCESS) { std::cerr<<"SigmOutput: "<<rtn<<std::endl; getchar(); }
	}

	real clwCalcCost(int p_n_neurons, real* p_correct)
	{
		clwInitLib();
		int p_layer = g_cl_Outputs.size()-1;
		real* pDer = clwMapPartDer(p_layer, p_n_neurons-1, CL_MAP_WRITE);
		real* Out = clwMapOutput(p_layer, p_n_neurons-1, CL_MAP_READ);

		real err=0;
		int size = p_n_neurons-1;
		for (int i=0; i<size; i++)
		{
			err += std::pow(p_correct[i] - Out[i], 2);
			pDer[i] = (Out[i] - p_correct[i]) * Out[i] * (1 - Out[i]);
		}
		clwUnmapPartDer(p_layer);
		clwUnmapOutput(p_layer);
		return err;
	}

	void clwCalcWCorr(int p_layer, int p_n_neurons_p, int p_n_neurons_a)
	{
		clwInitLib();
		cl_int rtn;
		rtn = clSetKernelArg(g_cl_Kernels[KERNEL_WCOR], 0, sizeof(cl_mem), &g_cl_WCorrec[p_layer]);
		rtn = clSetKernelArg(g_cl_Kernels[KERNEL_WCOR], 1, sizeof(cl_mem), &g_cl_PartDer[p_layer]);
		rtn = clSetKernelArg(g_cl_Kernels[KERNEL_WCOR], 2, sizeof(cl_mem), &g_cl_Outputs[p_layer-1]);
		rtn = clSetKernelArg(g_cl_Kernels[KERNEL_WCOR], 3, sizeof(cl_int), &p_n_neurons_p);
		size_t* workSize = new size_t[2]{(size_t)p_n_neurons_a-1, (size_t)p_n_neurons_p};

		rtn = clEnqueueNDRangeKernel(g_cl_Command_Queue, g_cl_Kernels[KERNEL_WCOR], 2, nullptr, workSize, nullptr, 0, nullptr, nullptr);
		if (rtn != CL_SUCCESS) { std::cerr<<"CalcWCorr: "<<rtn<<std::endl; getchar(); }
		#ifdef FINISH
		rtn = clFinish(g_cl_Command_Queue);
		#else
		rtn = clEnqueueBarrierWithWaitList(g_cl_Command_Queue, 0, nullptr, nullptr);
		#endif
		if (rtn != CL_SUCCESS) { std::cerr<<"CalcWCorr: "<<rtn<<std::endl; getchar(); }
	}

	void clwCalcPartDer(int p_layer, int p_n_neurons_a, int p_n_neurons_s)
	{
		//std::cerr<<"CalcPartDer: "<<p_layer<<", "<<p_n_neurons_a<<", "<<p_n_neurons_s<<std::endl;
		//POSSIBILE BUG IN QUESTA FUNZIONE!!!
		clwInitLib();
		int neuronsA = p_n_neurons_a;
		int neuronsS = p_n_neurons_s-1;
		clSetKernelArg(g_cl_Kernels[KERNEL_PDER], 0, sizeof(cl_mem), &g_cl_PartDer[p_layer]);
		clSetKernelArg(g_cl_Kernels[KERNEL_PDER], 1, sizeof(cl_mem), &g_cl_PartDer[p_layer+1]);
		clSetKernelArg(g_cl_Kernels[KERNEL_PDER], 2, sizeof(cl_mem), &g_cl_Weights[p_layer+1]);
		clSetKernelArg(g_cl_Kernels[KERNEL_PDER], 3, sizeof(cl_mem), &g_cl_Outputs[p_layer]);
		clSetKernelArg(g_cl_Kernels[KERNEL_PDER], 4, sizeof(cl_int), &neuronsA);
		clSetKernelArg(g_cl_Kernels[KERNEL_PDER], 5, sizeof(cl_int), &neuronsS);
		size_t size = neuronsA-1;
		cl_int rtn = clEnqueueNDRangeKernel(g_cl_Command_Queue, g_cl_Kernels[KERNEL_PDER], 1, nullptr, &size, nullptr, 0, nullptr, nullptr);
		if (rtn != CL_SUCCESS) { std::cerr<<"CalcParDer: "<<rtn<<std::endl; getchar(); }
		#ifdef FINISH
		rtn = clFinish(g_cl_Command_Queue);
		#else
		rtn = clEnqueueBarrierWithWaitList(g_cl_Command_Queue, 0, nullptr, nullptr);
		#endif
		if (rtn != CL_SUCCESS) { std::cerr<<"CalcPartDer: "<<rtn<<std::endl; getchar(); }
	}

	void clwApplyCorr(int p_layer, int p_n_neurons_p, int p_n_neurons_a, float p_eta)
	{
		clwInitLib();
		cl_int neuronsP = p_n_neurons_p;
		cl_int neuronsA = p_n_neurons_a-1;
		cl_float eta = p_eta;
		cl_int rtn;
		clSetKernelArg(g_cl_Kernels[KERNEL_APWC], 0, sizeof(cl_mem), &g_cl_Weights[p_layer]);
		clSetKernelArg(g_cl_Kernels[KERNEL_APWC], 1, sizeof(cl_mem), &g_cl_WCorrec[p_layer]);
		clSetKernelArg(g_cl_Kernels[KERNEL_APWC], 2, sizeof(cl_int), &neuronsP);
		clSetKernelArg(g_cl_Kernels[KERNEL_APWC], 3, sizeof(cl_float), &eta);
		size_t* size = new size_t[2]{(size_t)neuronsA, (size_t)neuronsP};

		rtn = clEnqueueNDRangeKernel(g_cl_Command_Queue, g_cl_Kernels[KERNEL_APWC], 2, nullptr, size, nullptr, 0, nullptr, nullptr);
		if (rtn != CL_SUCCESS) {std::cerr<<"ApplyCorr: "<<rtn<<std::endl; getchar();}
		#ifdef FINISH
		rtn = clFinish(g_cl_Command_Queue);
		#else
		rtn = clEnqueueBarrierWithWaitList(g_cl_Command_Queue, 0, nullptr, nullptr);
		#endif
		if (rtn != CL_SUCCESS) { std::cerr<<"ApplyCorr: "<<rtn<<std::endl; getchar(); }
	}

	void clwReadOutput(int p_n_neurons, real* p_output)
	{
		clwInitLib();
		cl_int res = clEnqueueReadBuffer(g_cl_Command_Queue, g_cl_Outputs.back(), true, 0, sizeof(real) * (p_n_neurons-1), p_output, 0, nullptr, nullptr);
		if (res != CL_SUCCESS) { std::cerr<<"ReadOutput: "<<res<<std::endl; getchar(); }
	}
}

void clwTerminate()
{
	clFinish(g_cl_Command_Queue);
	size_t bufN = g_cl_Outputs.size();
	clReleaseMemObject(g_cl_Outputs[0]);
	for (size_t i=1; i<bufN; i++)
	{
		clReleaseMemObject(g_cl_Outputs[i]);
		clReleaseMemObject(g_cl_Weights[i]);
		clReleaseMemObject(g_cl_WCorrec[i]);
		clReleaseMemObject(g_cl_PartDer[i]);
	}
	g_cl_Outputs.clear();
	g_cl_Weights.clear();
	g_cl_WCorrec.clear();
	g_cl_PartDer.clear();
	for (int i=0; i<NUM_KERNELS; i++)
		clReleaseKernel(g_cl_Kernels[i]);
	clReleaseCommandQueue(g_cl_Command_Queue);
	clReleaseDevice(g_cl_Device);
	clReleaseContext(g_cl_Context);
}
