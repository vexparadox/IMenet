all:
	g++ *.cpp ../lib/*.cpp -lenet -o output -g -std=c++20 -Wno-int-to-void-pointer-cast -Wno-format -lpthread
OSXstandalone:
	g++ *.cpp -o output -std=c++20 -Wno-switch /usr/local/lib/libenet.a -mmacosx-version-min=10.9