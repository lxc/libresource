CC = gcc
export LD_LIBRARY_PATH = `git rev-parse --show-toplevel`
CFLAGS = -g -Wall -Werror -I $(LD_LIBRARY_PATH) -std=gnu99 -L $(LD_LIBRARY_PATH)
REXE = cpu_test
MFILES = cpu_info.*

cpu_test: cpu_test.c 
	$(CC) $(CFLAGS) -o $@ $^ -lresource

clean:
	rm -rf $(MFILES) $(REXE)
