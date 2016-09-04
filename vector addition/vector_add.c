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
#include <CL/cl.h>

//오류 처리
#define CHECK_ERROR(err) \
	if(err!= CL_SUCCESS){ \
		printf("[%s:%d] OpenCL error %d\n", __FILE__, __LINE__, err); \
		exit(EXIT_FAILURE); \
	}



