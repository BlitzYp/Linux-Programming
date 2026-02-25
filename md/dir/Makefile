all:
	gcc -std=c99 -Wall -Wextra -Wno-deprecated-declarations md_dir.c -o md_dir -lcrypto

clean:
	rm ./md_dir

test:
	./md_dir

test_d_flag:
	./md_dir -d

test_h_flag:
	./md_dir -h

test_m_flag:
	./md_dir -m

test_md_flag:
	./md_dir -m -d
