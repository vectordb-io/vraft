CC:=g++
CFLAGS+=-c -Wall -g -std=c++17

OBJS_SERVER=server_main.o config.o vraft_rpc.grpc.pb.o vraft_rpc.pb.o status.o thread_pool.o timer.o grpc_server.o env.o node.o storage.o coding.o async_task_called.o async_task_call.o raft.o log.o util.o
INCLUDES_SERVER=-I./
LIBS_SERVER=-lglog -lprotobuf -lgrpc++ -lgrpc -lpthread -lleveldb
LFLAGS_SERVER=-L/usr/local/lib

OBJS_CLIENT=client_main.o vraft_rpc.grpc.pb.o vraft_rpc.pb.o status.o
INCLUDES_CLIENT=-I./
LIBS_CLIENT=-lglog -lprotobuf -lgrpc++ -lgrpc -lpthread -lleveldb
LFLAGS_CLIENT=-L/usr/local/lib

OBJS_THREAD_POOL_TEST=thread_pool_test.o thread_pool.o
INCLUDES_THREAD_POOL_TEST=-I./
LIBS_THREAD_POOL_TEST=-lglog -lprotobuf -lgrpc++ -lgrpc -lpthread -lleveldb
LFLAGS_THREAD_POOL_TEST=-L/usr/local/lib

cs: vraft_server vraft_client
	@echo ""
	@echo "success, go go go!"
	@echo ""

test: thread_pool_test 
	@echo ""
	@echo "make $<"
	@echo ""

thread_pool_test: $(OBJS_THREAD_POOL_TEST)
	$(CC) $^ -o $@ $(LIBS_THREAD_POOL_TEST) $(LFLAGS_THREAD_POOL_TEST)

vraft_server: $(OBJS_SERVER)
	$(CC) $^ -o $@ $(LIBS_SERVER) $(LFLAGS_SERVER)

vraft_client: $(OBJS_CLIENT)
	$(CC) $^ -o $@ $(LIBS_CLIENT) $(LFLAGS_CLIENT)

%.o: %.cc %.h
	$(CC) $< $(CFLAGS) -o $@

.PHONY:
proto:
	protoc -I. --cpp_out=. vraft_rpc.proto
	protoc -I. --grpc_out=. --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` vraft_rpc.proto

all:
	make proto && make cs && make test

clean:
	rm -rf *.o 
	rm -rf vraft_server
	rm -rf vraft_client
	rm -rf thread_pool_test

cleanall: clean
	rm -f vraft_rpc.pb.*
	rm -f vraft_rpc.grpc.pb.*



