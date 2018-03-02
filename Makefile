RAPIDJSON := -I src/libs/rapidjson/include
MINIZIP := src/libs/zlib/inflate.c \
			src/libs/zlib/inftrees.c \
			src/libs/zlib/inffast.c \
			src/libs/zlib/crc32.c \
			src/libs/zlib/deflate.c \
			src/libs/zlib/trees.c \
			src/libs/zlib/adler32.c \
			src/libs/zlib/zutil.c \
			src/libs/zlib/contrib/minizip/zip.c \
			src/libs/zlib/contrib/minizip/ioapi.c \
			src/libs/zlib/contrib/minizip/unzip.c
MINIZIP_O := inflate.o inffast.o zutil.o trees.o adler32.o zip.o deflate.o ioapi.o crc32.o unzip.o inftrees.o

build:
	gcc -c $(MINIZIP)
	g++ -g cli/*.cpp src/*.cpp -std=gnu++11 -lm $(RAPIDJSON) $(MINIZIP_O) -lcurl -o get

run_tests:
	rm -rf tests/.get/packages tests/.get/tmp
	gcc -c $(MINIZIP)
	g++ -g tests/*.cpp src/*.cpp -std=gnu++11 -lm $(RAPIDJSON) $(MINIZIP_O) -lcurl -o get_tests
	./get_tests

clean:
	rm *.o get get_tests