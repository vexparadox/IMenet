UNAME := $(shell uname)
ifeq ($(UNAME), Darwin)
	OSXFLAG = -mmacosx-version-min=10.9
endif
all:
	g++ *.cpp -lenet -o output -g -std=c++11 -Wno-int-to-void-pointer-cast -Wno-format -lpthread $(OSXFLAG)