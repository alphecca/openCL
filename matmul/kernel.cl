__kernel void mat_mul(__global float *A,
					__global float *B,
					__global float *C,
					const int NDIM){

    //TODO 동적 메모리 할당을 하려면 global memory이므로 host인 mat_mul.c에서 malloc()?
    
    int k,j;
    int i= get_global_id(0);
    float tmp=0.0f;
    for(j=0;j<NDIM;j++){
    	for(k=0;k<NDIM;k++)
    		tmp += A[i*NDIM+k]*B[k*NDIM+j];
    	barrier(CLK_LOCAL_MEM_FENCE);
    	C[i*NDIM+j] += tmp;
    }
}