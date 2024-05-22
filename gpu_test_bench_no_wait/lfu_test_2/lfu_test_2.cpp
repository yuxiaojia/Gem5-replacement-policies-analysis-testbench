#include "hip/hip_runtime.h"
#define HCC_ENABLE_PRINTF
#include <stdio.h>
#include <stdlib.h>

// define enough entries to fit all of addresses we want to access
#define CACHE_ENTRIES 512

// kernel code
// Access pattern: A, C, E, G, A, I, K, M, O, A
// Each letter represents a 64-byte address range.

// The [] indicate two different sets, and each set has four ways.
// [set0way0, set0way1, set0way2, set0way3],
// [set1way0, set1way1, set1way2, set1way3],
// If you have a 512B cache with 4-way associativity, and each cache
// line is 64B, with LFU replacement policy, you will observe:
// m, m, m, m, h, m, m, m, m, h  where 'm' means miss, and 'h' means
// hit.

// Explanation of this result:
// A, C, E, G are misses, now the cache stores
// ([A1, C1, E1, G1],[ , , ,]).The number after each letter
// is the count for the  accesses for the address range.
// A is a hit, and the cache now stores ([A2, C1, E1, G1],[ , , ,]).
// I searches a victim and selects C. Now the cache stores ([A2, I1, E1, G1],[ , , ,]).
// K searches a victim and selects I. Now the cache stores ([A2, K1, E1, G1],[ , , ,]).
// M searches a victim and selects K. Now the cache stores ([A2, M1, E1, G1],[ , , ,]).
// O searches a victim and selects M. Now the cache stores ([A2, O1, E1, G1],[ , , ,]).
// A hits.

__global__ void kernel(int * arr, uint64_t * dummy) {

  uint64_t a = 0, b = 0, c = 0, d = 0, e = 0, f = 0, g = 0, h = 0, i = 0, j = 0;
  asm volatile(

	       "s_waitcnt vmcnt(0) & lgkmcnt(0)\n\t"
	       "buffer_wbinvl1\n\t"
               "flat_load_dwordx2 %[out0], %[in1]\n\t"
               "flat_load_dwordx2 %[out2], %[in3]\n\t"
               "flat_load_dwordx2 %[out4], %[in5]\n\t"
               "flat_load_dwordx2 %[out6], %[in7]\n\t"
               "flat_load_dwordx2 %[out8], %[in9]\n\t"
               "flat_load_dwordx2 %[out10], %[in11]\n\t"
               "flat_load_dwordx2 %[out12], %[in13]\n\t"
               "flat_load_dwordx2 %[out14], %[in15]\n\t"
               "flat_load_dwordx2 %[out16], %[in17]\n\t"
               "flat_load_dwordx2 %[out18], %[in19]\n\t"
               "s_nop 0\n\t"
               : [out0]"=v"(a), [out2]"=v"(b), [out4]"=v"(c), [out6]"=v"(d), [out8]"=v"(e), [out10]"=v"(f), [out12]"=v"(g), [out14]"=v"(h), [out16]"=v"(i), [out18]"=v"(j)
	       : [in1]"v"((uint64_t *)&arr[0]), [in3]"v"((uint64_t *)&arr[32]), [in5]"v"((uint64_t *)&arr[64]), [in7]"v"((uint64_t *)&arr[96]), [in9]"v"((uint64_t *)&arr[1]),
		 [in11]"v"((uint64_t *)&arr[128]), [in13]"v"((uint64_t *)&arr[160]), [in15]"v"((uint64_t *)&arr[192]), [in17]"v"((uint64_t *)&arr[224]), [in19]"v"((uint64_t *)&arr[2])
               :"memory"
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
