CXX=g++

CXXFLAGS=-Wall -fno-strict-aliasing -O0 -ggdb -D_DEBUG -D_DBGMSG 

INCLUDE = -I. -Iinclude -ICommon -INetwork -IDatabase -ICardlib
LIBS=extlibs/libtinyredis.ra extlibs/libevwork.ra -lev -lhiredis -ljsoncpp -lrt


TARGET=pdk

SRC_FILES=$(wildcard *.cpp \
	Common/*.cpp	\
	Network/*.cpp	\
	Database/*.cpp \
	Cardlib/*.cpp)

OBJ_FILES=$(SRC_FILES:.cpp=.o)

$(TARGET):$(OBJ_FILES)
	$(CXX) -o $@ $(OBJ_FILES) $(CXXFLAGS) $(LIBS)

%.o : %.cpp
	g++ -c -o $@ $< $(CXXFLAGS) $(INCLUDE)

.PHONY: clean start stop restart 

clean:
	-rm -f $(TARGET)\
	   	$(OBJ_FILES)

start:
#	./$(TARGET)&
	./run.sh
stop:
	./stop.sh
restart:
	./restart.sh
