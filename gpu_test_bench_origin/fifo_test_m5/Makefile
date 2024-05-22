HIPOPTS = --amdgpu-target=gfx900,gfx906 --save-temps

HIP_PATH ?= /opt/rocm/hip

# By default, assume gem5 is in the same directory as this repo
GEM5_PATH ?= $(BASE_DIR)/../../gem5

# Needed for m5 ops
GEM5_CFLAGS := -I$(GEM5_PATH)/include
GEM5_FS_CFLAGS :=  -I$(GEM5_PATH)/util/m5/build/x86
M5_LDFLAGS += -L$(GEM5_PATH)/util/m5/build/x86/out -lm5

CC := $(HIP_PATH)/bin/hipcc

SRC = fifo_test_m5.cpp

EXE = fifo_test_m5


release:
	$(CC) $(HIPOPTS) $(SRC) -o $(EXE) $(CFLAGS) $(LDFLAGS)

clean:
	rm -f *.o *~ $(EXE)

run:
	./$(EXE)

profile:
	rocprof ./$(EXE)

events:
	rocprof  --events elapsed_cycles_sm ./$(EXE)
