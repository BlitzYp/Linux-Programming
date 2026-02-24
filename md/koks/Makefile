all:
	gcc md_koks.c -std=c99 -O3 -o md_koks

test:
	./md_koks < in.txt > out.txt

test_medium:
	./md_koks < test_medium.txt

test_large:
	./md_koks < test_large.txt

test_medium_checker:
	./md_koks < test_medium.txt | python3 checker.py 31512fb951883d7c19c28ed1538a245edd78e55f6ee80ac685f3891dc5ce55de

test_large_checker:
	./md_koks < test_large.txt | python3 checker.py c41428c3506306469facb7fe47534536f222ec492d77803db158c90561f96335

test_wrong:
	./md_koks < test_wrong.txt

