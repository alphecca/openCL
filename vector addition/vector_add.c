/*
 * vector_add.c
 *
 *  Created on: 2016. 9. 4.
 *      Author: 다윤
 *
 *      두 벡터를 더하는 프로그램
 *      두 벡터 A와 B를 GPU로 보낸 다음, 둘을 합한 벡터를 계산하고, 이를 다시 GPU에서 벡터 C로 가져온다.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>//NULL 사용
#include <CL/cl.h>

//오류 처리
#define CHECK_ERROR(err) \
	if(err!= CL_SUCCESS){ \
		printf("[%s:%d] OpenCL error %d\n", __FILE__, __LINE__, err); \
		exit(EXIT_FAILURE); \
	}

//소스 코드를 파일에서 읽어 들이기
char *get_source_code(const char *file_name, size_t *len){
	char *source_code;
	size_t length;
	FILE *file = fopen(file_name, "r");
	if(file == NULL){
		printf("[%s:%d] Failed to open %s\n", __FILE__, __LINE__, file_name);
		exit(EXIT_FAILURE);
	}

	fseek(file, 0, SEEK_END);
	length = (size_t)ftell(file);
	rewind(file);

	source_code = (char *)malloc(length+1);
	fread(source_code, length, 1, file);
	source_code[length] = '\0';

	fclose(file);

	*len = length;
	return source_code;
}

//main function
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

	err=clGetPlatformIDs(1, &platform, NULL);
	CHECK_ERROR(err);

	err=clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
	CHECK_ERROR(err);

	err=clCreateContext(NULL, 1, &device, NULL, NULL, &err);
	CHECK_ERROR(err);

	queue = clCreateCommandQueue(context, device, 0, &err);
	CHECK_ERROR(err);

	kernel_source = get_source_code("kernel.cl", &kernel_source_size);
	program = clCreateProgramWithSource(context, 1, (const char**)&kernel_source, &kernel_source_size, &err);
	CHEKC_ERROR(err);

	//컴파일 에러 메시지 출력
	err= clBuildProgram(program, 1, &device, "", NULL, NULL);
	if(err==CL_BUILD_PROGRAM_FAILURE){
		size_t log_size;
		char *log;

		err= clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
		CHECK_ERROR(err);

		log=(char*)malloc(log_size+1);
		err= clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
		log[log_size] = '\0';
		printf("Compile error:\n%s\n", log);
		free(log);
	}
	CHECK_ERROR(err);

	//이름이 vec_add인 커널 함수에 대한 커널 오브젝트 만들기
	kernel = clCreateKernel(program, "vec_add", &err);

	return 0;
}



