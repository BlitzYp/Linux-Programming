all:
	gcc -o md_mem md_mem.c
	gcc -o md_mem_100 md_mem_100.c

test_mem:
	./md_mem malloc >> mem.txt
	echo >> mem.txt
	./md_mem sbrk >> mem.txt
	echo >> mem.txt
	./md_mem mmap >> mem.txt
	echo >> mem.txt

test_time:
	(time ./md_mem_100 malloc -t) >> time.txt 2>&1
	echo >> time.txt
	(time ./md_mem_100 sbrk -t) >> time.txt 2>&1
	echo >> time.txt
	(time ./md_mem_100 mmap -t) >> time.txt 2>&1
	echo >> time.txt

test_time_bash:
	./test_time.sh >> time.txt

test_size:
	objdump -h md_mem_100 >> size.txt
	echo "SIZE INFO" >> size.txt
	size md_mem_100 >> size.txt

clean:
	rm md_mem md_mem_100 time.txt mem.txt size.txt