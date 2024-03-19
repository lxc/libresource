CC = cc
export LD_LIBRARY_PATH = `git rev-parse --show-toplevel`
CFLAGS = -g -Wall -Werror -I $(LD_LIBRARY_PATH) -std=gnu99 -L $(LD_LIBRARY_PATH) -lresource 
REXE = mem_test mem_test_cg
MFILES = mem_info.*

mem_test: mem_test.c 
	$(CC) $(CFLAGS) -o $@ $^

mem_test_cg: mem_test_cg.c 
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf $(MFILES) $(REXE)
