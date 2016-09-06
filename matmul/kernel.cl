#define BLOCK_SIZE 20
#define MATRIX_SIZE 100 * 100
	
#define BLOCK_DIMX 5 // Number of blocks in the x dimension

__kernel void mat_mul(__global float *A,
					__global float *B,
					__globat float *C,
					const int NDIM){
	// Matrix multiplication: C = A * B.

    // Block index
    int bx = get_group_id(0);
    int by = get_group_id(1);
	
	
    __global float *C = selectMatrixC + (bx/BLOCK_DIMX) * MATRIX_SIZE;
	    __global float *A = selectMatrixA + (bx/BLOCK_DIMX) * MATRIX_SIZE;
	    __global float *B = selectMatrixB + (bx/BLOCK_DIMX) * MATRIX_SIZE;
	
	
	
	    int tx = get_local_id(0);
	    int ty = get_local_id(1);
	
	    float Csub = 0;
	
	    // Identify the row and column of the C matrix to work on
	
	    int Row = (by * BLOCK_SIZE)  + ty;
	    int Col = ((bx %(BLOCK_DIMX)) * BLOCK_SIZE) + tx;
	
	    // Declaration of the local memory array As used to store the sub-matrix of A
	    __local float As[BLOCK_SIZE][BLOCK_SIZE];
	
	    // Declaration of the local memory array Bs used to store the sub-matrix of B
	    __local float Bs[BLOCK_SIZE][BLOCK_SIZE];
	
	    // Loop over all the sub-matrices of A and B required to compute the block sub-matrix
	    for (int m = 0; m < wA / BLOCK_SIZE; ++m) 
	    {
	
	        // Load the matrices from global memory to local memory. Each thread loads one   
	        //element of each matrix
	        As[ty][tx] = A[Row * wA + m * BLOCK_SIZE + tx];
	        Bs[ty][tx] = B[(m * BLOCK_SIZE + ty)*wA + Col];
	
	        // Synchronize to make sure the matrices are loaded
	        barrier(CLK_LOCAL_MEM_FENCE);
	
	        // Multiply the two matrices together each thread computes one element of the block 
	        //sub-matrix
	        for (int k = 0; k < BLOCK_SIZE; ++k)
	            Csub += As[ty][k] * Bs[k][tx];
	
	        // Synchronize to make sure that the preceding computation is done before loading 
	        //two new sub-matrices of A and B in the next iteration
	        barrier(CLK_LOCAL_MEM_FENCE);
	
	    }
	
	    // Write the block sub-matrix to device memory each thread writes one element
	    C[Row * wA + Col] = Csub;
	
}