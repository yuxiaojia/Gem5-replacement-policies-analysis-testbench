#include "hip/hip_runtime.h"
#define HCC_ENABLE_PRINTF
#include <stdio.h>
#include <stdlib.h>

// define enough entries to fit all of addresses we want to access
#define CACHE_ENTRIES 512

// kernel code
__global__ void kernel(int * arr, uint64_t * dummy) {
  asm volatile (
		/*
		  Ensure any initializing code is not being cached before code we care about.
		*/
    "s_waitcnt vmcnt(0) & lgkmcnt(0)\n\t"
		"buffer_wbinvl1\n\t"
		);
  // every array location is 4B wide, so given a cache of size N and associativity M, we can figure out which addresses map to which indices
  
  //a b c d a b e f c d
  volatile int a = arr[0];
  volatile int b = arr[32];
  volatile int c = arr[64];
  volatile int d = arr[96];
  volatile int e = arr[65]; // same cache line as 65
  volatile int f = arr[97]; // same cache line as 96
  volatile int g = arr[1]; // same cache line as 0
  volatile int h = arr[33]; // same cache line as 32
  volatile int i = arr[128];
  volatile int j = arr[66]; // same cache line as 64
  volatile int k = arr[2]; // same cache line as 0

  // dummy write to avoid optimizing out any accesses 
  //dummy[0] = a + b + c + d + e + f + g + h + i + j;
  asm volatile (// dummy nop to mark where code ends
		"s_nop 0\n\t"
		);
}

// host code
int main(){
    int *arr = (int *)calloc(sizeof(int), CACHE_ENTRIES);
    uint64_t *dummy = (uint64_t *)calloc(sizeof(uint64_t), 1);

    int *arr_g;
    uint64_t *dummy_g;

    hipMallocManaged(&arr_g, CACHE_ENTRIES*sizeof(int));
    hipMallocManaged(&dummy_g, CACHE_ENTRIES*sizeof(uint64_t));

    // initialize arr_g with arr
    hipMemcpy(arr_g, arr, CACHE_ENTRIES*sizeof(int), hipMemcpyHostToDevice);

    // just want 1 GPU thread to run our kernel
    hipLaunchKernelGGL(kernel, dim3(1), dim3(1), 0, 0, arr_g, dummy_g);

    // copy dummy value back to ensure compiler doesn't optimize out anything
    hipMemcpy(dummy, dummy_g, 1*sizeof(uint64_t), hipMemcpyDeviceToHost);
    printf("Dummy value = %lu\n", dummy[0]);

    hipFree(arr_g);
    hipFree(dummy_g);
    free(arr);
    free(dummy);
    return 0;
}
