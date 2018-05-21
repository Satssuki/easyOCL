#all:
#	clang  -O3 -std=c11 -o ./bin/gemm -c ./src/Opt_MatrixMultiplication.c  -I/opt/intel/opencl/include  -mf16c

	
BIN_DIR = ./bin
SRC_DIR = ./src
INC_DIR = /opt/intel/opencl/include
LIB_DIR = /opt/intel/opencl
OBJ_DIR = ./obj

SRC = $(wildcard ${SRC_DIR}/*.c)
OBJS = $(patsubst %.c,${OBJ_DIR}/%.o,$(notdir ${SRC}))

TARGET = main
BIN_TARGET = ${BIN_DIR}/${TARGET}

CC=clang
CXX = clang++
CXXFLAGS = -O3 -std=c11 -mf16c `pkg-config --cflags OpenCL`
LINKFLAGS= `pkg-config --libs OpenCL`

${BIN_TARGET} :${OBJS}
	$(CC) -o ${BIN_TARGET} ${OBJS} $(LINKFLAGS)

${OBJ_DIR}/%.o:${SRC_DIR}/%.c
	$(CC) $(CXXFLAGS) -o $@ -c $< 


all: ${BIN_TARGET}

clean:
	find $(OBJ) -name *.o -exec rm -rf {} \; 
	rm -rf $(BIN_TARGET)