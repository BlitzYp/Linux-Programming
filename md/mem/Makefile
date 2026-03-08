all:
	gcc -O2 -o md_mem md_mem.c
	gcc -O2 -o md_mem_100 md_mem_100.c

test:
	./md_mem malloc

test_100:
	./md_mem_100

clean:
	rm *.out md_mem md_mem_100