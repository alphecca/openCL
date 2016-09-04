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

	//프로그램 빌드 및 컴파일 에러 메시지 출력
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
	//@NOTE 커널 오브젝트는 프로그램이 빌드된 다음 만들어야 함
	kernel = clCreateKernel(program, "vec_add", &err);
	CHECK_ERROR(err);

	int *A = (int*)malloc(sizeof(int)*16384);
	int *B = (int*)malloc(sizeof(int)*16384);
	int *C = (int*)malloc(sizeof(int)*16384);
	int i;

	for(i=0;i<16384;i++){
		A[i] = rand()%100;
		B[i] = rand()%100;
	}

	cl_mem bufA, bufB, bufC;
	bufA = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(int)*16384, NULL, &err);
	CHECK_ERROR(err);
	bufB = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(int)*16384, NULL, &err);
	CHECK_ERROR(err);
	bufC = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(int)*16384, NULL, &err);
	CHECK_ERROR(err);

	err = clEnqueueWriteBuffer(queue, bufA, CL_FALSE, 0, sizeof(int)*16384, A, 0, NULL, NULL);
	CHECK_ERROR(err);
	err = clEnqueueWriteBuffer(queue, bufB, CL_FALSE, 0, sizeof(int)*16384, B, 0, NULL, NULL);
	CHECK_ERROR(err);
	//@NOTE blocking_write==CL_TRUE : 동기화를 의미. 버퍼쓰기가 완료된 다음에 return.
	//@NOTE blocking_write==CL_FALSE : 비동기화를 의미. 커맨드가 큐에 enqueue되자마자 return. 완료시점파악을 위해 이벤트 사용.

	//커널 인자 설정 : arg_value를 arg_index번 인자로 넘기기
	err=clSetKernelArg(kernel, 0, sizeof(cl_mem), &bufA);
	CHECK_ERROR(err);
	err=clSetKernelArg(kernel, 1, sizeof(cl_mem), &bufB);
	CHECK_ERROR(err);
	err=clSetKernelArg(kernel, 2, sizeof(cl_mem), &bufC);
	CHECK_ERROR(err);

	//커널 실행 : work_dim 차원의 커널 인덱스 공간을 만든다.
	size_t global_size = 16384;
	size_t local_size = 256;
	err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);
	CHECK_ERROR(err);

	//버퍼 읽기
	err = clEnqueueReadBuffer(queue, bufC, CL_TRUE, 0, sizeof(int)*16384, C, 0, NULL, NULL);
	CHECK_ERROR(err);

	for(i=0;i<16384; i++){
		if(A[i] +B[i] != C[i]){
			printf("Verification failed! A[%d] = %d, B[%d] = %d, C[%d] = %d\n", i, A[i], i, B[i], i, C[i]);
			break;
		}
	}
	if(i==16384)	printf("Verification success!\n");

	clReleaseMemObject(bufA);
	clReleaseMemObject(bufB);
	clReleaseMemObject(bufC);
	free(A);
	free(B);
	free(C);
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(queue);
	clReleaseContext(context);

	return 0;
}




