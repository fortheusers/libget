make:	*.cpp
	g++ *.cpp -std=gnu++11 -I rapidjson/include -I /usr/local/include -lm -lz -lminizip -lcurl -o get
