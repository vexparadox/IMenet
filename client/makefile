UNAME := $(shell uname)
ifeq ($(UNAME), Darwin)
	OSXFLAG = -mmacosx-version-min=10.9
endif
all:
	g++ *.cpp -lenet -o output -std=c++11 -Wno-switch -lpthread $(OSXFLAG)