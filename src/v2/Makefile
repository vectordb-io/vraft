CC:=g++
CFLAGS+=-c -Wall -g -std=c++17
INCLUDES=-I./ -I../../third_party/jsonxx.v0.9.2/include/

OBJS_SERVER=server_main.o config.o vraft_rpc.grpc.pb.o vraft_rpc.pb.o status.o thread_pool.o timer.o grpc_server.o env.o node.o storage.o coding.o async_task_called.o async_task_call.o raft.o log.o util.o async_req_manager.o
LIBS_SERVER=-lglog -lprotobuf -lgrpc++ -lgrpc -lpthread -lleveldb
LFLAGS_SERVER=-L/usr/local/lib

OBJS_CLIENT=client_main.o vraft_rpc.grpc.pb.o vraft_rpc.pb.o status.o
LIBS_CLIENT=-lglog -lprotobuf -lgrpc++ -lgrpc -lpthread -lleveldb
LFLAGS_CLIENT=-L/usr/local/lib

OBJS_THREAD_POOL_TEST=thread_pool_test.o thread_pool.o status.o
LIBS_THREAD_POOL_TEST=-lglog -lprotobuf -lgrpc++ -lgrpc -lpthread -lleveldb
LFLAGS_THREAD_POOL_TEST=-L/usr/local/lib

OBJS_CONFIG_TEST=config_test.o config.o status.o util.o
LIBS_CONFIG_TEST=-lglog -lprotobuf -lgrpc++ -lgrpc -lpthread -lleveldb
LFLAGS_CONFIG_TEST=-L/usr/local/lib

OBJS_LOG_TEST=log_test.o log.o status.o vraft_rpc.pb.o
LIBS_LOG_TEST=-lglog -lprotobuf -lgrpc++ -lgrpc -lpthread -lleveldb
LFLAGS_LOG_TEST=-L/usr/local/lib

OBJS_LOG_TEST2=log_test2.o log.o status.o vraft_rpc.pb.o
LIBS_LOG_TEST2=-lglog -lprotobuf -lgrpc++ -lgrpc -lpthread -lleveldb
LFLAGS_LOG_TEST2=-L/usr/local/lib

cs: vraft_server vraft_client
	@echo ""
	@echo "success, go go go!"
	@echo ""

test: thread_pool_test config_test log_test log_test2
	@echo ""
	@echo "make $<"
	@echo ""

log_test2: $(OBJS_LOG_TEST2)
	$(CC) $^ -o $@ $(LIBS_LOG_TEST2) $(LFLAGS_LOG_TEST2)

log_test: $(OBJS_LOG_TEST)
	$(CC) $^ -o $@ $(LIBS_LOG_TEST) $(LFLAGS_LOG_TEST)

config_test: $(OBJS_CONFIG_TEST)
	$(CC) $^ -o $@ $(LIBS_CONFIG_TEST) $(LFLAGS_CONFIG_TEST)

thread_pool_test: $(OBJS_THREAD_POOL_TEST)
	$(CC) $^ -o $@ $(LIBS_THREAD_POOL_TEST) $(LFLAGS_THREAD_POOL_TEST)

vraft_server: $(OBJS_SERVER)
	$(CC) $^ -o $@ $(LIBS_SERVER) $(LFLAGS_SERVER)

vraft_client: $(OBJS_CLIENT)
	$(CC) $^ -o $@ $(LIBS_CLIENT) $(LFLAGS_CLIENT)

%.o: %.cc %.h
	$(CC) $(INCLUDES) $(CFLAGS) -o $@ $<

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
	rm -rf config_test
	rm -rf log_test
	rm -rf log_test2

cleanall: clean
	rm -f vraft_rpc.pb.*
	rm -f vraft_rpc.grpc.pb.*



