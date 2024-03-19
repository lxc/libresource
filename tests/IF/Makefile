CC = gcc
export LD_LIBRARY_PATH = `git rev-parse --show-toplevel`
CFLAGS = -g -Wall -Werror -I $(LD_LIBRARY_PATH) -std=gnu99 -L $(LD_LIBRARY_PATH)
REXE = if_test
MFILES = if_info.*

if_test: if_test.c 
	$(CC) $(CFLAGS) -o $@ $^ -lresource

clean:
	rm -rf $(MFILES) $(REXE)
