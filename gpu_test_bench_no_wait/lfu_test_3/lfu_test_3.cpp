#include "hip/hip_runtime.h"
#define HCC_ENABLE_PRINTF
#include <stdio.h>
#include <stdlib.h>

// define enough entries to fit all of addresses we want to access
#define CACHE_ENTRIES 512

// kernel code
// Access pattern: A, A, C, C, C, E, G, G, I, E, E, E, E, K,
// A, A, A, A, M, G, G, G, G, O, C
// Each letter represents a 64-byte address range.

// The [] indicate two different sets, and each set has four ways.
// [set0way0, set0way1, set0way2, set0way3],
// [set1way0, set1way1, set1way2, set1way3],
// If you have a 512B cache with 4-way associativity, and each cache
// line is 64B, with LFU replacement policy, you will observe:
// m, h, m, h, h, m, m, h, m, m, h, h, h, m, m, h, h, h, m, m, h, h,
// h, m, m, where 'm' means miss, and 'h' means hit.

// Explanation of this result:
// After two A accesses, three C accesses, one E access and two G accesses,
// the cache stores ([A2,C3,E1,G2],[ , , ,]). The numbers after each letter are the
// counts of accesses to that address range.
// I searches a victim and selects E. Now the cache stores ([A2,C3,I1,G2],[ , , ,]).
// E searches a victim and selects I. Now the cache stores ([A2,C3,E1,G2],[ , , ,]).
// Three E accesses are hits. Now the cache stores ([A2,C3,E4,G2],[ , , ,]).
// K searches a victim and selects A. Now the cache stores ([K1,C3,E4,G2],[ , , ,]).
// A searches a victim and selects K. Now the cache stores ([A1,C3,E4,G2],[ , , ,]).
// Three A accesses are hits. Now the cache stores ([A4,C3,E4,G2],[ , , ,]).
// M searches a victim and selects G. Now the cache stores ([A4,C3,E4,M1],[ , , ,]).
// G searches a victim and selects M. Now the cache stores ([A4,C3,E4,G1],[ , , ,]).
// Three G accesses are hits. Now the cache stores ([A4,C3,E4,G4],[ , , ,]).
// O searches a victim and selects C. Now the cache stores ([A4,O1,E4,G4],[ , , ,]).
// C searches a victim and selects O. Now the cache stores ([A4,C1,E4,G4],[ , , ,]).

__global__ void kernel(int * arr, uint64_t * dummy) {

  uint64_t a = 0, b = 0, c = 0, d = 0, e = 0, f = 0, g = 0, h = 0, i = 0, j = 0, k = 0, l = 0, m = 0, n = 0, o = 0, p = 0, q = 0, r = 0, s = 0, t = 0, u = 0, v = 0, w = 0, x = 0, y = 0;
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
               "flat_load_dwordx2 %[out20], %[in21]\n\t"
               "flat_load_dwordx2 %[out22], %[in23]\n\t"
               "flat_load_dwordx2 %[out24], %[in25]\n\t"
               "flat_load_dwordx2 %[out26], %[in27]\n\t"
               "flat_load_dwordx2 %[out28], %[in29]\n\t"
               "flat_load_dwordx2 %[out30], %[in31]\n\t"
               "flat_load_dwordx2 %[out32], %[in33]\n\t"
               "flat_load_dwordx2 %[out34], %[in35]\n\t"
               "flat_load_dwordx2 %[out36], %[in37]\n\t"
               "flat_load_dwordx2 %[out38], %[in39]\n\t"
               "flat_load_dwordx2 %[out40], %[in41]\n\t"
               "flat_load_dwordx2 %[out42], %[in43]\n\t"
               "flat_load_dwordx2 %[out44], %[in45]\n\t"
               "flat_load_dwordx2 %[out46], %[in47]\n\t"
               "flat_load_dwordx2 %[out48], %[in49]\n\t"
               "s_nop 0\n\t"
               : [out0]"=v"(a), [out2]"=v"(b), [out4]"=v"(c), [out6]"=v"(d), [out8]"=v"(e), [out10]"=v"(f), [out12]"=v"(g), [out14]"=v"(h), [out16]"=v"(i), [out18]"=v"(j), 
               [out20]"=v"(k), [out22]"=v"(l), [out24]"=v"(m), [out26]"=v"(n), [out28]"=v"(o), [out30]"=v"(p), [out32]"=v"(q), [out34]"=v"(r), [out36]"=v"(s), [out38]"=v"(t),
               [out40]"=v"(u), [out42]"=v"(v), [out44]"=v"(w), [out46]"=v"(x), [out48]"=v"(y)
	       : [in1]"v"((uint64_t *)&arr[0]), [in3]"v"((uint64_t *)&arr[1]), [in5]"v"((uint64_t *)&arr[32]), [in7]"v"((uint64_t *)&arr[33]), [in9]"v"((uint64_t *)&arr[34]),
		 [in11]"v"((uint64_t *)&arr[64]), [in13]"v"((uint64_t *)&arr[96]), [in15]"v"((uint64_t *)&arr[97]), [in17]"v"((uint64_t *)&arr[128]), [in19]"v"((uint64_t *)&arr[65])
     , [in21]"v"((uint64_t *)&arr[66]), [in23]"v"((uint64_t *)&arr[67]), [in25]"v"((uint64_t *)&arr[68]), [in27]"v"((uint64_t *)&arr[160]), [in29]"v"((uint64_t *)&arr[2])
     , [in31]"v"((uint64_t *)&arr[3]), [in33]"v"((uint64_t *)&arr[4]), [in35]"v"((uint64_t *)&arr[5]), [in37]"v"((uint64_t *)&arr[192]), [in39]"v"((uint64_t *)&arr[98])
     , [in41]"v"((uint64_t *)&arr[99]), [in43]"v"((uint64_t *)&arr[100]), [in45]"v"((uint64_t *)&arr[101]), [in47]"v"((uint64_t *)&arr[224]), [in49]"v"((uint64_t *)&arr[35])
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
