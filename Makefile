ifneq ($(SOURCES),)
	# we're in a chesto-style lib
	LIBGET  		:= $(CURDIR)/libs/get/src
	LIBGET_REPOS	:= $(LIBGET)/repos
else
	# we are in the libget folder already
	LIBGET := ./src
	APP_VERSION := 2.0-$(shell git describe --tags --always --dirty)
endif

RAPIDJSON   := $(LIBGET)/libs/rapidjson/include
MINIZIP     := $(LIBGET)/libs/minizip
TINYXML     := $(LIBGET)/libs/tinyxml

SOURCES     += $(LIBGET) $(LIBGET_REPOS) $(MINIZIP) $(TINYXML)
INCLUDES    += $(RAPIDJSON) $(MINIZIP) $(TINYXML)

VPATH       += $(LIBGET) $(LIBGET_REPOS) $(MINIZIP) $(TINYXML)

CFLAGS      += -Wall -Wextra -Werror

CFLAGS      += -DNETWORK
LDFLAGS     += -lcurl

MINIZIP_O   :=  zip.o ioapi.o unzip.o

ifeq ($(LIBGET),./src)
build:
	gcc -c $(MINIZIP)/*.c
	g++ -g cli/*.cpp src/*.cpp src/repos/*.cpp -std=gnu++20 -lm -I $(RAPIDJSON) $(MINIZIP_O) -I $(MINIZIP) -lz -lcurl -o get -DAPP_VERSION=\"$(APP_VERSION)\"

run_tests:
	rm -rf tests/.get/packages tests/.get/tmp tests/.get/repos
	gcc -c $(MINIZIP)/*.c
	g++ -g tests/*.cpp src/*.cpp src/repos/*.cpp -std=gnu++20 -lm -L/usr/lib -I $(RAPIDJSON) $(MINIZIP_O) -I $(MINIZIP) -lz -lcurl -o get_tests
	@echo "Starting test HTTP server with range request support..."
	@cd tests/server && python3 -m RangeHTTPServer > /dev/null 2>&1 & echo $$! > /tmp/libget_test_server.pid
	@sleep 2
	@echo "Running tests..."
	@./get_tests; TEST_RESULT=$$?; \
	if [ -f /tmp/libget_test_server.pid ]; then \
		echo "Stopping test HTTP server (PID $$(cat /tmp/libget_test_server.pid))..."; \
		kill $$(cat /tmp/libget_test_server.pid) 2>/dev/null || true; \
		rm /tmp/libget_test_server.pid; \
	fi; \
	exit $$TEST_RESULT

clean:
	rm *.o get get_tests
endif
