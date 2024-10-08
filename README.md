# Gem5 replacement policies analysis

Previously we have fully tested the replacement policies on CPU MI Example coherence, and the commit can be found at [github](https://github.com/gem5/gem5/commit/bd319560605f1e3eebf828efd7e06206874d6515) or [googlesource](https://gem5-review.googlesource.com/c/public/gem5/+/60389). Below are how to run the tests for CPU:

``` bash
git clone https://github.com/gem5/gem5.git
cd gem5
git checkout origin/develop -b develop
git checkout bd31956
python3 which scons build/X86_MI_example/gem5.opt -j9
cd tests
# first time run
./main.py run -j 9 -t 240 gem5/replacement-policies # -t indicates how many threads are used to run tests
# skip build after first time run
./main.py run --skip-build -t 240 gem5/replacement-policies
```

We will then use gem5 to analyze the benefits of replacement policies on GPU last level caches, and we will test on VEGA_X86 model. 

## Installation

Reference from [square](https://gem5.googlesource.com/public/gem5-resources/+/refs/heads/stable/src/gpu/square) test website, details about docker usage can be found there

### Building the gem5 GPU
All the changes needed for using this [testbench](https://github.com/yuxiaojia/Testbench) have been merged into develop branch.\
The changes can also be found from pull requests below:
1. [gpu-compute,mem-ruby: Add RubyHitMiss flag for TCP and TCC cache](https://github.com/gem5/gem5/pull/1260)
2. [gpu-compute, util-m5: add GPU kernel exit events](https://github.com/gem5/gem5/pull/1217)
3. [gpu-compute: Added functions to choose replacement policies for GPU](https://github.com/gem5/gem5/pull/1213)
4. [configs: Add replacement policy options for GPUFS](https://github.com/gem5/gem5/pull/1230)
```bash
git clone https://github.com/gem5/gem5.git
cd gem5
git checkout origin/develop -b develop
docker run --volume $(pwd):$(pwd) -w $(pwd) gcr.io/gem5-test/gcn-gpu:latest scons build/VEGA_X86/gem5.opt -j 8
```

### Compiling the tests
All testbench for the gpu testing is in https://github.com/yuxiaojia/Testbench.
1. gpu_test_bench_origin: With waitcnt between each instructions
2. gpu_test_bench_fewer_wait: With limited waitcnt that produces the same results as gpu_test_bench_origin
3. gpu_test_bench_no_wait: With no waitcnt between instructions that might produce different results as gpu_test_bench_origin
4. Add glc to bypass the L1 cache if want to test in L2 cahce

Compiling in [M5ops](https://www.gem5.org/documentation/general_docs/m5ops/) need to put the benchmarks in the same directory that includes the gem5 folder

```bash
cd test_folder
docker run --volume $(pwd):$(pwd) -w $(pwd) gcr.io/gem5-test/gcn-gpu:latest make -f test_make_file
```

### Running the tests
Running benchmarks with different replacement policies for TCP and TCC caches

1. First running with debug flag GPUKernelInfo to get kernel1 start time and complete time as debug start time and debug end time
```bash
#To get --debug-start time and --debug-end time
docker run --volume $(pwd):$(pwd) -w $(pwd) gcr.io/gem5-test/gcn-gpu:v22-1 build/VEGA_X86/gem5.opt --debug-flag=GPUKernelInfo configs/example/apu_se.py -n 3 --dgpu --gfx-version=gfx900 
--num-compute-units=4 --cu-per-sa=4 --num-gpu-complex=1 --reg-alloc-policy=dynamic --num-tccs=8 --num-dirs=64 --mem-size=16GB --mem-type=HBM_2000_4H_1x64 --vreg-file-size=16384 
--sreg-file-size=800 --tcc-size=4MB --gpu-clock=1801MHz --ruby-clock=1000MHz --vrf_lm_bus_latency=6 --mem-req-latency=69 --mem-resp-latency=69 --mandatory_queue_latency=1 
--max-cu-tokens=160 --max-coalesces-per-cycle=10 --sqc-size=16kB --tcp-size=4MB --scalar-mem-req-latency=28 --TCP_latency=4 --tcp-assoc=16 --tcp-num-banks=16 --TCC_latency=121 
--tcc-assoc=16 --tcc-tag-access-latency=1 --tcc-data-access-latency=1 --tcp-rp=FIFORP --WB_L2 --tcc-rp=FIFORP -c test_name
```

2. Running with debug start time and debug end time to get partial traces, and chooses debug flags like RubyHitMiss, ProtocolTrace, GPUKernelInfo, GPUExec, GPUALL, GPUCommandProc for more detailed traces
```bash
#To get detailed information
docker run --volume $(pwd):$(pwd) -w $(pwd) gcr.io/gem5-test/gcn-gpu:v22-1 build/VEGA_X86/gem5.opt --debug-flag=(e.x.RubyHitMiss,ProtocolTrace, GPUKernelInfo,GPUExec,GPUALL,GPUCommandProc) --debug-start=(kernel1_launch_time) 
--debug-end=(kernel1_complete_time) configs/example/apu_se.py -n 3 --dgpu --gfx-version=gfx900 --num-compute-units=4 --cu-per-sa=4 --num-gpu-complex=1 --reg-alloc-policy=dynamic 
--num-tccs=8 --num-dirs=64 --mem-size=16GB --mem-type=HBM_2000_4H_1x64 --vreg-file-size=16384 --sreg-file-size=800 --tcc-size=4MB --gpu-clock=1801MHz --ruby-clock=1000MHz 
--vrf_lm_bus_latency=6 --mem-req-latency=69 --mem-resp-latency=69 --mandatory_queue_latency=1 --max-cu-tokens=160 --max-coalesces-per-cycle=10 --sqc-size=16kB --tcp-size=4MB 
--scalar-mem-req-latency=28 --TCP_latency=4 --tcp-assoc=16 --tcp-num-banks=16 --TCC_latency=121 --tcc-assoc=16 --tcc-tag-access-latency=1 --tcc-data-access-latency=1 
--tcp-rp=FIFORP --WB_L2 --tcc-rp=FIFORP -c test_name
```

## Usage and further steps

### Running in real GPU
Real GPU information in eldin https://gpuopen.com/wp-content/uploads/2019/08/RDNA_Architecture_public.pdf
#### Add rocprof into the path
```bash
echo $SHELL
vim ~/.bashrc
```
add "export PATH=/opt/rocm/bin:$PATH" into the file to add rocm into the path
```bash
source ~/.bashrc
ls /opt/rocm/bin
```

#### Run benchmarks with rocprof
Run [benchmarks](https://github.com/yuxiaojia/real_gpu_benchmark) in real GPU with [rocprof](https://github.com/ROCm/rocprofiler/blob/rocm-4.0.x/doc/rocprof.md#6publicly-available-counters-and-metrics) in eldin machine to reverse engeering the replacement policies that are used in real GPU
```bash
rocprof -i input.txt application_name
```
Sample input.txt file:
```bash
   # Perf counters group 1
   # pmc : Wavefronts VALUInsts SALUInsts SFetchInsts MemUnitStalled
   # Perf counters group 2
   # pmc : TCC_HIT[0], TCC_MISS[0]
   # TCC_MISS[0], TCC_MISS[1]
   pmc : TCC_HIT_sum, TCC_MISS_sum,
   pmc: TCC_MISS[0], TCC_MISS[1], TCC_MISS[2], TCC_MISS[3],
   pmc: TCC_MISS[4],TCC_MISS[5],TCC_MISS[6],TCC_MISS[7],
   pmc: TCC_MISS[8],TCC_MISS[9],TCC_MISS[10],TCC_MISS[11],
   pmc: TCC_MISS[12],TCC_MISS[13],TCC_MISS[14],TCC_MISS[15],
   pmc: TCC_HIT[0], TCC_HIT[1], TCC_HIT[2], TCC_HIT[3],
   pmc: TCC_HIT[4],TCC_HIT[5],TCC_HIT[6],TCC_HIT[7],
   pmc: TCC_HIT[8],TCC_HIT[9],TCC_HIT[10],TCC_HIT[11],
   pmc: TCC_HIT[12],TCC_HIT[13],TCC_HIT[14],TCC_HIT[15],

   range: 
   gpu: 0 1
   kernel: 
```
### Running more complex benchmarks
Running [Rodinia, Pannotia](https://github.com/hal-uw/gem5-gpu-benchmark-suite/tree/main/condor) and more complex benchmarks to further test the benefits of replacement policies
#### How to use Condor
Use [Condor](https://chtc.cs.wisc.edu/) for running benchmarks in parallel

### Hint
#### Docker gcc version
Docker image might encounter error if the gcc version is outdate
```bash
Error: gcc version 10 or newer required
```
Fix by creating a new image:
```bash
cd $GEM5_HEAD/util/dockerfiles/gcn-gpu/
docker build -t <desiredImageName> .
```
Replace previous docker with 
```bash
docker.io/library/<desiredImageName>
```

