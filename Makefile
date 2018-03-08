CC = gcc
CFLAGS = -g -Wall -Werror -fPIC
DEPS = resource.h res_impl.h resmem.h resnet.h
OBJ = resource.o resmem.o resnet.o
LIB = libresource.so
TEST = test
RM = rm -rf
CP = cp

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: $(OBJ)
	$(CC) -shared -o $(LIB) $^ $(CFLAGS)

.PHONY : clean
clean:
	$(RM) $(LIB) $(OBJ) $(TEST)
