/*
 ============================================================================
 Name        : host.c
 Author      : 김다윤
 Version     :
 Copyright   :
 Description : The host program which does the following,
 	 	 	 	 get one platform randomly
 	 	 	 	 get one GPU device randomly
 	 	 	 	 make a context
 	 	 	 	 make a command-queue
 	 	 	 	 call kernel source code from kernel.cl
 	 	 	 	 make program object
 	 	 	 	 build program
 	 	 	 	 print error message for compile error
 	 	 	 	 make kernel object ( assume that kernel func. name as "my_kernel")
 	 	 	 	 release all objects (kernel object, program object, command-queue, context)
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
//#include <cl.h>

#define CHECK_ERROR(err) \
	if(err!=CL_SUCCESS){ \
		printf("[%s:%d] OpenCL error %d\n", __FILE__, __LINE__, err); \
	}

char *get_source_code(const char *file_name, size_t *len){
	char *source_code;
	size_t length;
	FILE *file = fopen(file_name, "r");
	if(file==NULL){
		printf("[%s:%d] Failed to open %s\n", __FILE__, __LINE__, file_name);
		exit(EXIT_FAILURE);
	}
	fseek(file, 0, SEEK_END);
	length = (size_t)ftell(file);
	rewind(file);

	source_code = (char *) malloc(length+1);
	fread(source_code, length, 1, file);
	source_code[length] = '\0';

	fclose(file);

	*len = length;

	return source_code;
}

int main(){
	cl_platform_id platform;
	cl_device_id device;
	cl_context context;
	cl_command_queue queue;
	cl_program program;
	char *kernel_source;
	size_t kernel_source_size;
	cl_kernel kernel;
	cl_int err;

	err= clGetPlatformsIDs(1, &platform, NULL);
	CHECK_ERROR(err);

	err=clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
	CHECK_ERROR(err);

	context=clCreateContext(NULL, 1, &device, NULL, NULL, &err);
	CHECK_ERROR(err);

	queue = clCreatCommandQueue(context, device, 0, &err);
	CHECK_ERROR(err);

	kernel_source = get_source_code("kernel.cl", &kernel_source_size);
	program = clCreateProgramWithSource(context, 1, (const char**)&kernel_source, &kernel_source_size, &err);
	CHECK_ERROR(err);

	err = clBuildProgram(program, 1, &device, "", NULL, NULL);
	if(err == CL_BUILD_PROGRAM_FAILURE){
		size_t log_size;
		char *log;

		err= clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
		CHECK_ERROR(err);

		log=(char*)malloc(log_size+1);
		err=clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
		CHECK_ERROR(err);

		log[log_size] = '\0';
		printf("Compiler error:\n%s\n",log);
		free(log);
	}

	CHECK_ERROR(err);

	kernel=clCreatKernel(program, "my_kernel", &err);
	CHECK_ERROR(err);

	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(queue);
	clReleaseContext(context);

	printf("Finished!\n");

	return 0;
}

