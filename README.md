# Gem5 replacement policies analysis

Previously we have fully tested the replacement policies on CPU MI Example coherence (https://gem5-review.googlesource.com/c/public/gem5/+/60389). Below are how to run the tests for CPU:

``` bash
git clone https://gem5.googlesource.com/public/gem5
cd gem5
git checkout origin/develop -b develop
git fetch https://gem5.googlesource.com/public/gem5 refs/changes/89/60389/16 && git checkout -b change-60389 FETCH_HEAD
python3 which scons build/X86_MI_example/gem5.opt -j9
./main.py run -j9 -t 240 gem5/replacement-policies
./main.py run --skip-build -t 240 gem5/replacement-policies
```

We will then use gem5 to analyze the benefits of replacement policies on GPU last level caches, and we will test on VEGA_X86 model. 

## Installation

Reference from [square](https://gem5.googlesource.com/public/gem5-resources/+/refs/heads/stable/src/gpu/square) test website, details about docker usage can be found there

### Building the gem5 GPU
The latest update is in https://github.com/gem5/gem5/compare/develop...yuxiaojia:gem5:develop
```bash
git clone https://gem5.googlesource.com/public/gem5
cd gem5
git checkout origin/develop -b develop
git chechout 26d951a
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

2. Running with debug start time and debug end time to get partial traces, and chooses debug flags like RubyHitMiss, GPUExec, GPUALL, GPUCommandProc for more detailed traces
```bash
#To get detailed information
docker run --volume $(pwd):$(pwd) -w $(pwd) gcr.io/gem5-test/gcn-gpu:v22-1 build/VEGA_X86/gem5.opt --debug-flag=(e.x.RubyHitMiss,GPUExec,GPUALL,GPUCommandProc) --debug-start=(kernel1_launch_time) 
--debug-end=(kernel1_complete_time) configs/example/apu_se.py -n 3 --dgpu --gfx-version=gfx900 --num-compute-units=4 --cu-per-sa=4 --num-gpu-complex=1 --reg-alloc-policy=dynamic 
--num-tccs=8 --num-dirs=64 --mem-size=16GB --mem-type=HBM_2000_4H_1x64 --vreg-file-size=16384 --sreg-file-size=800 --tcc-size=4MB --gpu-clock=1801MHz --ruby-clock=1000MHz 
--vrf_lm_bus_latency=6 --mem-req-latency=69 --mem-resp-latency=69 --mandatory_queue_latency=1 --max-cu-tokens=160 --max-coalesces-per-cycle=10 --sqc-size=16kB --tcp-size=4MB 
--scalar-mem-req-latency=28 --TCP_latency=4 --tcp-assoc=16 --tcp-num-banks=16 --TCC_latency=121 --tcc-assoc=16 --tcc-tag-access-latency=1 --tcc-data-access-latency=1 
--tcp-rp=FIFORP --WB_L2 --tcc-rp=FIFORP -c test_name
```

## Usage and further steps

### Running in real GPU
#### Add rocprof into the path
Real GPU information in eldin https://gpuopen.com/wp-content/uploads/2019/08/RDNA_Architecture_public.pdf
```bash
echo $SHELL
vim ~/.bashrc
source ~/.bashrc
ls /opt/rocm/bin
```

#### Run benchmarks with rocprof
Run [benchmarks](https://github.com/yuxiaojia/real_gpu_benchmark) in real GPU with [rocprof](https://github.com/ROCm/rocprofiler/blob/rocm-4.0.x/doc/rocprof.md#6publicly-available-counters-and-metrics) in eldin machine to reverse engeering the replacement policies that are used in real GPU
```bash
rocprof -i input.txt application_name
```
### Running more complex benchmarks
Running [rodinia, panotia](https://github.com/hal-uw/gem5-gpu-benchmark-suite/tree/main/condor) and more complex benchmarks to further test the benefits of replacement policies
#### How to use Condor
Use [Condor](https://chtc.cs.wisc.edu/) for running benchmarks in parallel

