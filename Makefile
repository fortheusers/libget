make:	src/*.cpp
	g++ -g src/*.cpp -std=gnu++11 -I src/libs/rapidjson/include -I /usr/local/include -lm -lz -lminizip -lcurl -o get
