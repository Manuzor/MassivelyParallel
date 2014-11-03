/**********************************************************************
Copyright ©2013 Advanced Micro Devices, Inc. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

•	Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
•	Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or
 other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************/

// For clarity,error checking has been omitted.

#include <CL/cl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>

#define SUCCESS 0
#define FAILURE 1

using namespace std;

#include "OpenCLMgr.h"

int summe(cl_int *input, int size, OpenCLMgr& mgr)
{
	cl_int status;
	int result;

	int clsize = (size+511)/512*512;  // next multiple of 512

	cl_mem inputBuffer = clCreateBuffer(mgr.context, CL_MEM_READ_ONLY, 512 * clsize * sizeof(cl_int),(void *) input, NULL);
	status = clEnqueueWriteBuffer(mgr.commandQueue, inputBuffer, CL_TRUE, 0, size * sizeof(cl_int), input, 0, NULL, NULL);
	CHECK_SUCCESS("Error: writing buffer!")
	if (size<clsize) 
	{
		cl_int tmp[512] = {0};
		status = clEnqueueWriteBuffer(mgr.commandQueue, inputBuffer, CL_TRUE, size * sizeof(cl_int), (clsize-size)*sizeof(cl_int), &tmp, 0, NULL, NULL);
		CHECK_SUCCESS("Error: writing buffer!")
	}

	int outsize = clsize/512;
	cl_int *output = new cl_int[outsize];
	cl_mem outputBuffer = clCreateBuffer(mgr.context, CL_MEM_WRITE_ONLY , outsize * sizeof(cl_int), NULL, NULL);

	// Set kernel arguments.
	status = clSetKernelArg(mgr.summe_kernel, 0, sizeof(cl_mem), (void *)&inputBuffer);
	CHECK_SUCCESS("Error: setting kernel argument 1!")
	status = clSetKernelArg(mgr.summe_kernel, 1, sizeof(cl_mem), (void *)&outputBuffer);
	CHECK_SUCCESS("Error: setting kernel argument 2!")
	
	// Run the kernel.
	size_t global_work_size[1] = {clsize/2};
	size_t local_work_size[1] = {256};
	status = clEnqueueNDRangeKernel(mgr.commandQueue, mgr.summe_kernel, 1, NULL, global_work_size, local_work_size, 0, NULL, NULL);
	CHECK_SUCCESS("Error: enqueuing kernel!")

	// Read the output back to host memory.
	status = clEnqueueReadBuffer(mgr.commandQueue, outputBuffer, CL_TRUE, 0, outsize * sizeof(cl_int), output, 0, NULL, NULL);
	CHECK_SUCCESS("Error: sreading buffer!")


	// release buffers
	status = clReleaseMemObject(inputBuffer);		
	status = clReleaseMemObject(outputBuffer);

	result=0;
	for (int i=0 ; i<outsize ; i++)
		result += output[i];

	delete [] output;

	return result;
}

int main(int argc, char* argv[])
{
	OpenCLMgr mgr;

	// Initial input,output for the host and create memory objects for the kernel
	cl_int input[1200];
	int size = 1200;
	for (int i=0 ; i<size ; i++)
		input[i] = 1;

	// call function
	int result = summe(input, size, mgr);
	
	cout << "Summe:" << result << endl;

	std::cout<<"Passed!\n";
	return SUCCESS;
}