ifneq ($(TOPDIR),)
	# we're in a chesto-style lib
	LIBGET  := $(TOPDIR)/libs/get/src
else
	# we are in the libget folder already
	LIBGET := ./src
endif

RAPIDJSON   := $(LIBGET)/libs/rapidjson/include
MINIZIP     := $(LIBGET)/libs/minizip
TINYXML     := $(LIBGET)/libs/tinyxml

export SOURCES     += $(LIBGET) $(MINIZIP) $(TINYXML)
export INCLUDES    += $(RAPIDJSON) $(MINIZIP) $(TINYXML)

export VPATH       += $(LIBGET) $(MINIZIP) $(TINYXML)

export CFLAGS      += -DNETWORK
export LDFLAGS     += -lcurl

# use C linker for all C files
ifeq ($(strip $(CPPFILES)),)
	export LD	:=	$(CC)
else
	export LD	:=	$(CXX)
endif

MINIZIP_O   :=  zip.o ioapi.o unzip.o

ifeq ($(TOPDIR),)
build:
	gcc -c $(MINIZIP)/*.c
	g++ -g cli/*.cpp src/*.cpp -std=gnu++11 -lm -I $(RAPIDJSON) $(MINIZIP_O) -I $(MINIZIP) -lz -lcurl -o get

run_tests:
	rm -rf tests/.get/packages tests/.get/tmp
	gcc -c $(MINIZIP)/*.c
	g++ -g tests/*.cpp src/*.cpp -std=gnu++11 -lm -lssl -lcrypto -L/usr/lib -L/usr/local/Cellar//openssl@1.1/1.1.0g/lib/ -I/usr/local/Cellar//openssl@1.1/1.1.0g/include -I $(RAPIDJSON) $(MINIZIP_O) -I $(MINIZIP) -lz -lcurl -o get_tests
	cd tests/server && python3 -m http.server &
	export SERVERD=$!
	sleep 2
	./get_tests

clean:
	rm *.o get get_tests
endif