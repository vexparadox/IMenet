all:
	g++ *.cpp ../lib/*.cpp -lenet -o output -g -std=c++20 -Wno-switch -lpthread 
OSXstandalone:
	g++ *.cpp -o output -std=c++20 -Wno-switch /usr/local/lib/libenet.a -mmacosx-version-min=10.9