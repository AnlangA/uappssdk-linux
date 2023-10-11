CC = gcc
CPP = g++
LD = g++

SRCS = $(wildcard *.c)

OBJS = $(patsubst %c, %o, $(SRCS))

SRCS_CPP = $(wildcard *.cpp)

OBJS_CPP = $(patsubst %cpp, %o, $(SRCS_CPP))

TARGET = uapps.bin

.PHONY:all clean

all: $(TARGET)


$(TARGET): $(OBJS) $(OBJS_CPP)
	$(LD) -o $@ $^

$(OBJS):
%.o:%.c
	$(CC) -c -g $^
$(OBJS_CPP):
%.o:%.cpp
	$(CPP) -c -g $^
clean:
	rm -f $(OBJS) $(OBJS_CPP) $(TARGET)
