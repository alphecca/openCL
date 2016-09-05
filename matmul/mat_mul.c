#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <CL/cl.h>
#include "timers.h"

#define NDIM    2048

/*오류 처리*/
#define CHECK_ERROR(err) \
	if(err!= CL_SUCCESS){ \
		printf("[%s:%d] OpenCL error %d\n", __FILE__, __LINE__, err); \
		exit(EXIT_FAILURE); \
	}

float a[NDIM][NDIM];
float b[NDIM][NDIM];
float c[NDIM][NDIM];

int print_matrix = 0;
int validation = 0;


void mat_mul( float c[NDIM][NDIM], float a[NDIM][NDIM], float b[NDIM][NDIM] )
{
	//declaration
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


	int i, j, k;
	
	// C = AB
	for( i = 0; i < NDIM; i++ )
	{
		for( j = 0; j < NDIM; j++ )
		{
			for( k = 0; k < NDIM; k++ )
			{
				c[i][j] += a[i][k] * b[k][j];
			}
		}
	}
}

/************************** DO NOT TOUCH BELOW HERE ******************************/

void check_mat_mul( float c[NDIM][NDIM], float a[NDIM][NDIM], float b[NDIM][NDIM] )
{
	int i, j, k;
	float sum;
	int validated = 1;

	printf("Validating the result..\n");
	
	// C = AB
	for( i = 0; i < NDIM; i++ )
	{
		for( j = 0; j < NDIM; j++ )
		{
			sum = 0;
			for( k = 0; k < NDIM; k++ )
			{
				sum += a[i][k] * b[k][j];
			}

			if( c[i][j] != sum )
			{
				printf("c[%d][%d] is differ(value=%lf correct_value=%lf)!!\n", i, j, c[i][j], sum );
				validated = 0;
			}
		}
	}

	printf("Validation : ");
	if( validated )
		printf("SUCCESSFUL.\n");
	else
		printf("FAILED.\n");
}

void print_mat( float mat[NDIM][NDIM] )
{
	int i, j;

	for( i = 0; i < NDIM; i++ )
	{
		for( j = 0; j < NDIM; j++ )
		{
			printf("%8.2lf ", mat[i][j]);
		}
		printf("\n");
	}
}

void print_help(const char* prog_name)
{
	printf("Usage: %s [-pvh]\n", prog_name );
	printf("\n");
	printf("OPTIONS\n");
	printf("  -p : print matrix data.\n");
	printf("  -v : validate matrix multiplication.\n");
	printf("  -h : print this page.\n");
}

void parse_opt(int argc, char** argv)
{
	int opt;

	while( (opt = getopt(argc, argv, "pvhikjs:")) != -1 )
	{
		switch(opt)
		{
		case 'p':
			// print matrix data.
			print_matrix = 1;
			break;

		case 'v':
			// validation
			validation = 1;
			break;

		case 'h':
		default:
			print_help(argv[0]);
			exit(0);
			break;
		}
	}
}

int main(int argc, char** argv)
{
	int i, j, k = 1;

	parse_opt( argc, argv );

	for( i = 0; i < NDIM; i++ )
	{
		for( j = 0; j < NDIM; j++ )
		{
			a[i][j] = k;
			b[i][j] = k;
			k++;
		}
	}

	timer_start(1);
	mat_mul( c, a, b );
	timer_stop(1);

	printf("Time elapsed : %lf sec\n", timer_read(1));


	if( validation )
		check_mat_mul( c, a, b );

	if( print_matrix )
	{
		printf("MATRIX A: \n");
		print_mat(a);

		printf("MATRIX B: \n");
		print_mat(b);

		printf("MATRIX C: \n");
		print_mat(c);
	}

	return 0;
}
