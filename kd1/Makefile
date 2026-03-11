CC=gcc
CFLAGS=-Wall -Wextra -Werror -std=c99 -O2
TARGET=kd1

.PHONY: all test test_buff clean re

# Uzbuvē programmu un obligatos translācijas failus.
all: $(TARGET) one zero

$(TARGET): kd1.c
	$(CC) $(CFLAGS) -o $(TARGET) kd1.c

# Vienibas translācijas fails, visi baiti paliek nemainiti.
one:
	@python3 -c "import sys; data=bytes(range(1,256)); sys.stdout.buffer.write(b'\x00'+data+b'\x00'+data)" > one

# Nulles translācijas fails, visi baiti tulkojas uz 0x00.
zero:
	@python3 -c "import sys; data=bytes(range(1,256)); sys.stdout.buffer.write(b'\x00'+data+b'\x00'+b'\x00'*255+b'\x00')" > zero

# Palaiž pamata testus visiem galvenajiem režīmiem.
# Python tiek lietots, lai precizi ģenerētu binarus failus ar vajadzīgo baitu saturu.
test: $(TARGET) one zero
	@echo "==========================================="
	@echo "XOR tests..."
	@printf 'ABC\n' > test.in
	@./$(TARGET) test.in -o out.bin
	@python3 -c "import sys; sys.stdout.buffer.write(bytes([0xC1,0xC2,0xC3,0x8A]))" > expected.bin
	@cmp out.bin expected.bin && echo "XOR tests OK" || (echo "XOR tests FAILED"; exit 1)
	@echo "==========================================="

	@echo "Translēšanas tests..."
	@printf 'XabcXABCY' > tran
	@printf 'abcXcab\n' > test.in
	@./$(TARGET) -t tran test.in -o out.txt
	@printf 'ABCYCAB\n' > expected.txt
	@cmp out.txt expected.txt && echo "Translēšanas tests OK" || (echo "Translēšanas tests FAILED"; exit 1)
	@echo "==========================================="

	@echo "Translēšanas tests ar newline separatoru..."
	@printf '\nabc\nABC\n' > tran_nl
	@printf 'abc\ncab\n' > test.in
	@./$(TARGET) -t tran_nl test.in -o out_nl.txt
	@printf 'ABC\nCAB\n' > expected_nl.txt
	@cmp out_nl.txt expected_nl.txt && echo "Newline translēšanas tests OK" || (echo "Newline translēšanas tests FAILED"; exit 1)
	@echo "==========================================="

	@echo "One translēšanas tests..."
	@python3 -c "import sys; sys.stdout.buffer.write(bytes([0,1,2,65,255]))" > test.in
	@./$(TARGET) -t one test.in -o out_one.bin
	@cmp test.in out_one.bin && echo "One translēšanas tests OK" || (echo "One translēšanas tests FAILED"; exit 1)
	@echo "==========================================="

	@echo "Zero translēšanas tests..."
	@python3 -c "import sys; sys.stdout.buffer.write(bytes([0,1,2,65,255]))" > test.in
	@./$(TARGET) -t zero test.in -o out_zero.bin
	@python3 -c "import sys; sys.stdout.buffer.write(b'\x00'*5)" > expected_zero.bin
	@cmp out_zero.bin expected_zero.bin && echo "Zero translēšanas tests OK" || (echo "Zero translēšanas tests FAILED"; exit 1)
	@echo "==========================================="

	@echo "Šifrēšanas tests..."
	@python3 -c "import sys; sys.stdout.buffer.write(bytes(255-i for i in range(256)))" > chip
	@printf 'Hello!\n' > test.in
	@./$(TARGET) -s chip test.in > out2.txt
	@python3 -c "import sys; data=b'Hello!\n'; sys.stdout.buffer.write(bytes(255-b for b in data))" > expected_chip.bin
	@cmp out2.txt expected_chip.bin && echo "Šifrēšanas tests OK" || (echo "Šifrēšanas tests FAILED"; exit 1)
	@echo "==========================================="

	@echo "Visi testi izieti!!!!"
	@echo "==========================================="


# Priekš B daļas
# BUF_SIZE=1 dod praktiski nebuferizetu režimu.
test_buff:
	@for size in 1 16 64 256 1024 4096 8192 16384 32768; do \
		echo "==========================================="; \
		echo "BUF_SIZE=$$size"; \
		$(CC) $(CFLAGS) -DBUF_SIZE=$$size -o kd1a kd1_test.c; \
		time -f "real=%e user=%U sys=%S" ./kd1a lielais.in > /dev/null; \
	done
	@echo "==========================================="

clean:
	rm -f $(TARGET) kd1a out.bin expected.bin out.txt expected.txt out_nl.txt expected_nl.txt out2.txt expected_chip.bin tran_nl out_one.bin out_zero.bin expected_zero.bin

re: clean all
