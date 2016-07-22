#SRC = vaserver.cpp vanet.cpp predict.c
PROTO_DIR = protobuf
PROTO_NAME = vanet_pb
#OBJ = vanet.o $(PROTO_DIR)/$(PROTO_NAME).pb.o caffe_pred.o vid_utils.o video.o
BUILD_OBJ = vaserver.o vanet.o $(PROTO_DIR)/$(PROTO_NAME).pb.o vid_utils.o video.o classifier.o task_handlers.o algo/algo.o algo/node.o
TEST_OBJ = vatest.o	vanet.o $(PROTO_DIR)/$(PROTO_NAME).pb.o caffe_pred.o vid_utils.o video.o classifier.o


TARGETS = va
CCPP = g++
CC = gcc -w
CFLAGS = -c -w
CPPFLAGS = -c -ggdb3 -O0 -std=c++11
CAFFE = /home/user/src/caffe
CUDA  = /usr/local/cuda
BLAS = /usr/src/OpenBLAS
INCLUDE_DIRS += $(CAFFE)/include /usr/include $(CUDA)/include
INCLUDE_DIRS += $(BLAS)
INCLUDE_DIRS += .
LIBRARY_DIRS = $(CAFFE)/build/lib $(BLAS) /usr/lib $(CUDA)/lib /usr/lib/boost
LIBRARIES := caffe m stdc++ rt openblas protobuf glog  opencv_highgui opencv_core opencv_imgproc boost_regex pthread
INC_DIRS += $(foreach includedir,$(INCLUDE_DIRS),-I$(includedir))
LIB_DIRS += $(foreach librarydir,$(LIBRARY_DIRS),-L$(librarydir))
LIBS += $(foreach library,$(LIBRARIES),-l$(library))

.SUFIXES: .cpp .o

.cpp.o:
	$(CCPP) $(CPPFLAGS) $< $(INC_DIRS) -o $@

all : $(TARGETS)

va : $(BUILD_OBJ)
	$(CCPP) $(BUILD_OBJ) $(LIB_DIRS) $(LIBS) -o $@

test : $(TEST_OBJ)
	$(CCPP) $(TEST_OBJ) $(LIB_DIRS) $(LIBS) -o $@

proto:
	cd $(PROTO_DIR); \
		rm $(PROTO_NAME).* || true; \
		wget https://raw.githubusercontent.com/zblassman/va_protobuf/master/vanet_pb.proto; \
		protoc $(PROTO_NAME).proto --cpp_out=.; \
		mv $(PROTO_NAME).pb.cc $(PROTO_NAME).pb.cpp

clean:
	rm *.o || true
	rm protobuf/*.o || true
	rm algo/*.o || true
