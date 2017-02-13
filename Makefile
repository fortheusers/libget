make:	*.hpp *.cpp
	g++ -c *.hpp *.cpp -I /usr/local/include
	ld *.o -o get -lm -lz -lminizip -lstdc++ -lc++ -lcrt1.o
