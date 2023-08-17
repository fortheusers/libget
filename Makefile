ifneq ($(SOURCES),)
	# we're in a chesto-style lib
	LIBGET  := $(CURDIR)/libs/get/src
else
	# we are in the libget folder already
	LIBGET := ./src
	APP_VERSION := $(shell git describe --tags --always --dirty)
endif

RAPIDJSON   := $(LIBGET)/libs/rapidjson/include
MINIZIP     := $(LIBGET)/libs/minizip
TINYXML     := $(LIBGET)/libs/tinyxml

SOURCES     += $(LIBGET) $(MINIZIP) $(TINYXML)
INCLUDES    += $(RAPIDJSON) $(MINIZIP) $(TINYXML)

VPATH       += $(LIBGET) $(MINIZIP) $(TINYXML)

CFLAGS      += -DNETWORK
LDFLAGS     += -lcurl

MINIZIP_O   :=  zip.o ioapi.o unzip.o

ifeq ($(LIBGET),./src)
build:
	gcc -c $(MINIZIP)/*.c
	g++ -g cli/*.cpp src/*.cpp -std=gnu++20 -lm -I $(RAPIDJSON) $(MINIZIP_O) -I $(MINIZIP) -lz -lcurl -o get -DAPP_VERSION=\"$(APP_VERSION)\"

run_tests:
	rm -rf tests/.get/packages tests/.get/tmp
	gcc -c $(MINIZIP)/*.c
	g++ -g tests/*.cpp src/*.cpp -std=gnu++20 -lm -lssl -lcrypto -L/usr/lib -L/usr/local/Cellar//openssl@1.1/1.1.0g/lib/ -I/usr/local/Cellar//openssl@1.1/1.1.0g/include -I $(RAPIDJSON) $(MINIZIP_O) -I $(MINIZIP) -lz -lcurl -o get_tests
	cd tests/server && python3 -m http.server &
	export SERVERD=$!
	sleep 2
	./get_tests

clean:
	rm *.o get get_tests
endif