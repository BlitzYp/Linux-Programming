CC=gcc
CFLAGS=-std=c11 -Wall -Wextra

all: ld6_mem

ld6_mem: ld6_mem.c
	$(CC) $(CFLAGS) ld6_mem.c -o ld6_mem

heap: ld6_mem
	./ld6_mem one > one.txt
	./ld6_mem many > many.txt

time: ld6_mem
	(time ./ld6_mem 100) > 100mb.txt 2>&1
	(time ./ld6_mem 100 -t) > 100mb_touch.txt 2>&1

vm: ld6_mem
	(./ld6_mem 100; sleep 2) & echo $$! > .pid
	sleep 1
	cat /proc/$$(cat .pid)/status | grep -E 'VmSize|VmRSS' > ps.txt
	kill $$(cat .pid) 2>/dev/null || true
	rm -f .pid
	(./ld6_mem 100 -t; sleep 2) & echo $$! > .pid
	sleep 1
	cat /proc/$$(cat .pid)/status | grep -E 'VmSize|VmRSS' > ps_touch.txt
	kill $$(cat .pid) 2>/dev/null || true
	rm -f .pid

stack: ld6_mem
	: > stack.txt
	./ld6_mem stack 1 -t >> stack.txt 2>&1 || true
	./ld6_mem stack 2 -t >> stack.txt 2>&1 || true
	./ld6_mem stack 4 -t >> stack.txt 2>&1 || true
	./ld6_mem stack 6 -t >> stack.txt 2>&1 || true
	./ld6_mem stack 8 -t >> stack.txt 2>&1 || true
	./ld6_mem stack 10 -t >> stack.txt 2>&1 || true
	./ld6_mem stack 12 -t >> stack.txt 2>&1 || true
	./ld6_mem stack 16 -t >> stack.txt 2>&1 || true

test: heap time vm stack

clean:
	rm -f ld6_mem one.txt many.txt 100mb.txt 100mb_touch.txt ps.txt ps_touch.txt stack.txt .pid
