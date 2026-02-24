all:
	gcc pd2.c -o pd2

test:
	./pd2 test.in test.out
	cmp test.out test.in && echo "Strādā!" || echo "Atšķiras!"

test_big:
	./pd2 test-big.in test.out
	cmp test.out test-big.in && echo "Strādā!" || echo "Atšķiras!"

test_fail:
	./pd2 test2.in test2.out

test_exists:
	./pd2 test.in exists.out
	cmp exists.out test.in && echo "Strādā!" || echo "Atšķiras!"

test_time:
	@bash -c "time ./pd2 test-big.in test.out"

gentest:
	dd if=/dev/urandom of=test.in count=2048
	dd if=/dev/urandom of=test-big.in count=204800

clean:
	rm -f pd2 test.out test.in test-big.in
