CC=gcc
CFLAGS=-Wall -Wextra -Werror -std=c11 -O2
TARGET=kd1

.PHONY: all test clean re

all: $(TARGET)

$(TARGET): kd1.c
	$(CC) $(CFLAGS) -o $(TARGET) kd1.c

test: $(TARGET)
	@echo "XOR tests..."
	@printf 'ABC\n' > test.in
	@./$(TARGET) test.in -o out.bin
	@python3 -c "import sys; sys.stdout.buffer.write(bytes([0xC1,0xC2,0xC3,0x8A]))" > expected.bin
	@cmp out.bin expected.bin && echo "XOR tests OK" || (echo "XOR tests FAILED"; exit 1)

	@echo "Translēšanas tests..."
	@printf 'XabcXABCY' > tran
	@printf 'abcXcab\n' > test.in
	@./$(TARGET) -t tran test.in -o out.txt
	@printf 'ABCYCAB\n' > expected.txt
	@cmp out.txt expected.txt && echo "Translēšanas tests OK" || (echo "Translēšanas tests FAILED"; exit 1)

	@echo "Šifrēšanas tests..."
	@python3 -c "import sys; sys.stdout.buffer.write(bytes(range(256)))" > chip
	@printf 'Hello!\n' > test.in
	@./$(TARGET) -s chip test.in > out2.txt
	@cmp test.in out2.txt && echo "Šifrēšanas tests OK" || (echo "Šifrēšanas tests FAILED"; exit 1)

	@echo "All tests passed."

clean:
	rm -f $(TARGET) out.bin expected.bin out.txt expected.txt out2.txt

re: clean all