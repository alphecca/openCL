/*
 * info.c
 * 플랫폼과 디바이스의 정보를 출력한다.
 *
 *  Created on: 2016. 9. 4.
 *      Author: 다윤
 */
#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>

#define CHECK_ERROR(err) \
	if( err != CL_SUCCESS) { \
		printf("[%s:%d] OpenCL error %d\n", __FILE__, __LINE__, err); \
	}
int main() {
	//declaration
	cl_uint num_platforms;
	cl_platform_id *platforms;
	cl_uint num_devices;
	cl_device_id *devices;
	char str[1024];
	cl_device_type device_type;
	size_t max_work_group_size;
	cl_ulong global_mem_size;
	cl_ulong local_mem_size;
	cl_ulong max_mem_alloc_size;
	cl_uint p, d;
	cl_int err;

	//body
	//1.platform 개수 확인
	err = clGetPlatformIDs(0, NULL, &num_platforms);
	CHECK_ERROR(err);

	//2.platform ID num_platforms개 얻어오기
	platforms = (cl_platform_id*) malloc(sizeof(cl_platform_id)*num_platforms);
	clGetPlatformIDs(num_platforms, platforms, NULL);

	/*
	 * ex3. platform ID 아무거나 1개 얻어오기
	 * cl_platform_id platform;
	 * clGetPlatformIDs(1, &platform, NULL);
	 *
	 * ex4. platform ID 아무거나 4개 얻어오기(개수가 4개 미만이면 그만큼만)
	 * cl_platform_id platforms[4];
	 * cl_uint num_platforms;
	 * clGetPlatformIDs(4, platforms, &num_platforms);
	 * if(num_platforms <4){
	 * 	printf("Only %u platforms exist.", num_platforms);
	 * }
	 */

	//3. platform Info 얻어오기
	printf("Number of platforms: %u\n\n", num_platforms);
	for(p=0;p<num_platforms;p++){
		printf("platform: %u\n", p);

		err = clGetPlatformInfo(platforms[p], CL_PLATFORM_NAME, 1024, str, NULL);
		CHECK_ERROR(err);
		printf("- CL_PLATFORM_NAME : %s\n",str);

		err = clGetPlatformInfo(platforms[p], CL_PLATFORM_VENDOR, 1024, str, NULL);
		CHECK_ERROR(err);
		printf("- CL_PLATFORM_VENDOR : %s\n\n",str);

		//4. device 개수 얻어오기
		err = clGetDeviceIDs(platforms[p], CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);
		CHECK_ERROR(err);
		printf("Number of devices: %u\n\n", num_devices);

		//5. device ID num_devices개 얻어오기
		devices = (cl_device_id*)malloc(sizeof(cl_device_id) * num_devices);
		err = clGetDeviceIDs(platforms[p], CL_DEVICE_TYPE_ALL, num_devices, devices, NULL);
		CHECK_ERROR(err);

		//6. device Info 얻어오기
		for(d=0;d<num_devices; d++){
			printf("device : %u\n",d);

			err = clGetDeviceInfo(devices[d], CL_DEVICE_TYPE, sizeof(cl_device_type), &device_type, NULL);
			CHECK_ERROR(err);
			printf("- CL_DEVICE_TYPE :");
			if(device_type & CL_DEVICE_TYPE_CPU) printf("CL_DEVICE_TYPE_CPU");
			if(device_type & CL_DEVICE_TYPE_GPU) printf("CL_DEVICE_TYPE_GPU");
			if(device_type & CL_DEVICE_TYPE_ACCELERATOR) printf("CL_DEVICE_TYPE_ACCELERATOR");
			if(device_type & CL_DEVICE_TYPE_DEFAULT) printf("CL_DEVICE_TYPE_DEFAULT");
			if(device_type & CL_DEVICE_TYPE_CUSTOM) printf("CL_DEVICE_TYPE_CUSTOM");
			printf("\n");

			err= clGetDeviceInfo(devices[d], CL_DEVICE_NAME, 1024, str, NULL);
			CHECK_ERROR(err);
			printf("- CL_DEVICE_NAME : %s\n", str);

			err = clGetDeviceInfo(devices[d], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &max_work_group_size, NULL);
			CHECK_ERROR(err);
			printf("- CL_DEVICE_MAX_WORK_GROUP_SIZE : %u\n", max_work_group_size);

			err = clGetDeviceInfo(devices[d], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &global_mem_size, NULL);
			CHECK_ERROR(err);
			printf("- CL_DEVICE_GLOBAL_MEM_SIZE : %u\n", global_mem_size);

			err= clGetDeviceInfo(devices[d], CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong), &local_mem_size, NULL);
			CHECK_ERROR(err);
			printf("- CL_DEVICE_LOCAL_MEM_SIZE : %u\n", local_mem_size);

			err= clGetDeviceInfo(devices[d], CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong), &max_mem_alloc_size, NULL);
			CHECK_ERROR(err);
			printf("- CL_DEVICE_MAX_MEM_ALLOC_SIZE : %u\n", max_mem_alloc_size);

		}//end of device for loop

		free(devices);

	}//end of platform for loop

	free(platforms);

	return 0;
}//end of main


