all: gtest leveldb libuv

gtest:
	mkdir -p googletest/build
	cd googletest/build && cmake .. -DCMAKE_CXX_FLAGS="-fPIC" && make -j4 && cd -

leveldb:
	mkdir -p leveldb/build
	cd leveldb/build && cmake .. -DCMAKE_CXX_FLAGS="-fPIC" && make -j4 && cd -

libuv:
	cd libuv && sh autogen.sh && ./configure CFLAGS="-fPIC" CXXFLAGS="-fPIC" && make -j4

clean:
	rm -rf googletest/build
	rm -rf leveldb/build
	cd libuv && make clean && cd -

.PHONY: all gtest leveldb libuv