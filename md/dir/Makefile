CC := gcc
CFLAGS := -std=c99 -Wall -Wextra -Wno-deprecated-declarations
LDLIBS := -lcrypto

BIN := md_dir
SRC := md_dir.c

CASES_DIR := cases
CASE_DEFAULT := $(CASES_DIR)/default
CASE_DATE := $(CASES_DIR)/date
CASE_MD5 := $(CASES_DIR)/md5
EXPECTED_DIR := expected
TMP_DIR := .tmp

.PHONY: all clean clean_cases clean_tmp exec test test_d_flag test_m_flag test_h_flag \
	cases expected test_cases case_default case_date case_date_d case_md5 case_md5_m case_md5_dm \
	check_case_default check_case_date check_case_date_d check_case_md5 check_case_md5_m check_case_md5_dm \
	check_cases

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

clean:
	rm -f $(BIN)

clean_cases:
	rm -rf $(CASES_DIR)

clean_tmp:
	rm -rf $(TMP_DIR)

exec: all
	./$(BIN)

test: all
	./$(BIN)

test_d_flag: all
	./$(BIN) -d

test_m_flag: all
	./$(BIN) -m

test_h_flag: all
	./$(BIN) -h

cases:
	rm -rf $(CASES_DIR)
	mkdir -p $(CASE_DEFAULT)/a $(CASE_DEFAULT)/b
	printf 'hello\n' > $(CASE_DEFAULT)/a/dup.txt
	printf 'world\n' > $(CASE_DEFAULT)/b/dup.txt
	printf 'unique-file\n' > $(CASE_DEFAULT)/only.txt
	ln -s a/dup.txt $(CASE_DEFAULT)/dup_link.txt
	touch -d '2024-02-01 09:00:00' $(CASE_DEFAULT)/a/dup.txt
	touch -d '2024-02-01 09:00:00' $(CASE_DEFAULT)/b/dup.txt
	mkdir -p $(CASE_DATE)/same1 $(CASE_DATE)/same2 $(CASE_DATE)/diff1 $(CASE_DATE)/diff2
	printf 'keep-me\n' > $(CASE_DATE)/same1/keep.txt
	cp $(CASE_DATE)/same1/keep.txt $(CASE_DATE)/same2/keep.txt
	printf 'date-test\n' > $(CASE_DATE)/diff1/date.txt
	cp $(CASE_DATE)/diff1/date.txt $(CASE_DATE)/diff2/date.txt
	touch -d '2024-01-01 10:00:00' $(CASE_DATE)/same1/keep.txt
	touch -d '2024-01-01 10:00:00' $(CASE_DATE)/same2/keep.txt
	touch -d '2024-01-02 10:00:00' $(CASE_DATE)/diff1/date.txt
	touch -d '2024-01-03 10:00:00' $(CASE_DATE)/diff2/date.txt
	mkdir -p $(CASE_MD5)/x $(CASE_MD5)/y $(CASE_MD5)/k1 $(CASE_MD5)/k2 $(CASE_MD5)/d1 $(CASE_MD5)/d2
	printf 'same-content-for-md5\n' > $(CASE_MD5)/x/alpha.txt
	cp $(CASE_MD5)/x/alpha.txt $(CASE_MD5)/y/beta.bin
	touch -d '2024-02-10 11:00:00' $(CASE_MD5)/x/alpha.txt
	touch -d '2024-02-10 11:00:00' $(CASE_MD5)/y/beta.bin
	printf 'md5-and-date\n' > $(CASE_MD5)/k1/one.dat
	cp $(CASE_MD5)/k1/one.dat $(CASE_MD5)/k2/two.dat
	touch -d '2024-03-01 12:00:00' $(CASE_MD5)/k1/one.dat
	touch -d '2024-03-01 12:00:00' $(CASE_MD5)/k2/two.dat
	printf 'md5-same-diff-time\n' > $(CASE_MD5)/d1/left.bin
	cp $(CASE_MD5)/d1/left.bin $(CASE_MD5)/d2/right.bin
	touch -d '2024-04-01 08:00:00' $(CASE_MD5)/d1/left.bin
	touch -d '2024-04-02 08:00:00' $(CASE_MD5)/d2/right.bin

expected:
	@ls $(EXPECTED_DIR)/*.norm >/dev/null

case_default: all cases
	cd $(CASE_DEFAULT) && ../../$(BIN)

case_date: all cases
	cd $(CASE_DATE) && ../../$(BIN)

case_date_d: all cases
	cd $(CASE_DATE) && ../../$(BIN) -d

case_md5: all cases
	cd $(CASE_MD5) && ../../$(BIN)

case_md5_m: all cases
	cd $(CASE_MD5) && ../../$(BIN) -m

case_md5_dm: all cases
	cd $(CASE_MD5) && ../../$(BIN) -d -m

test_cases: case_default case_date case_date_d case_md5_m case_md5_dm

check_case_default: all cases expected
	mkdir -p $(TMP_DIR)
	(cd $(CASE_DEFAULT) && ../../$(BIN)) > $(TMP_DIR)/case_default.out
	awk '/^=== /{key=$$4 "|" $$5; print "G|" key; cur=key; next} NF==0{next} cur!=""{print "P|" cur "|" $$0}' $(TMP_DIR)/case_default.out | LC_ALL=C sort > $(TMP_DIR)/case_default.norm
	@if diff -u $(EXPECTED_DIR)/case_default.norm $(TMP_DIR)/case_default.norm; then \
		echo "PASS: check_case_default"; \
	else \
		echo "FAIL: check_case_default"; \
		exit 1; \
	fi

check_case_date: all cases expected
	mkdir -p $(TMP_DIR)
	(cd $(CASE_DATE) && ../../$(BIN)) > $(TMP_DIR)/case_date.out
	awk '/^=== /{key=$$4 "|" $$5; print "G|" key; cur=key; next} NF==0{next} cur!=""{print "P|" cur "|" $$0}' $(TMP_DIR)/case_date.out | LC_ALL=C sort > $(TMP_DIR)/case_date.norm
	@if diff -u $(EXPECTED_DIR)/case_date.norm $(TMP_DIR)/case_date.norm; then \
		echo "PASS: check_case_date"; \
	else \
		echo "FAIL: check_case_date"; \
		exit 1; \
	fi

check_case_date_d: all cases expected
	mkdir -p $(TMP_DIR)
	(cd $(CASE_DATE) && ../../$(BIN) -d) > $(TMP_DIR)/case_date_d.out
	awk '/^=== /{key=$$4 "|" $$5; print "G|" key; cur=key; next} NF==0{next} cur!=""{print "P|" cur "|" $$0}' $(TMP_DIR)/case_date_d.out | LC_ALL=C sort > $(TMP_DIR)/case_date_d.norm
	@if diff -u $(EXPECTED_DIR)/case_date_d.norm $(TMP_DIR)/case_date_d.norm; then \
		echo "PASS: check_case_date_d"; \
	else \
		echo "FAIL: check_case_date_d"; \
		exit 1; \
	fi

check_case_md5: all cases expected
	mkdir -p $(TMP_DIR)
	(cd $(CASE_MD5) && ../../$(BIN)) > $(TMP_DIR)/case_md5.out
	awk '/^=== /{key=$$4 "|" $$5; print "G|" key; cur=key; next} NF==0{next} cur!=""{print "P|" cur "|" $$0}' $(TMP_DIR)/case_md5.out | LC_ALL=C sort > $(TMP_DIR)/case_md5.norm
	@if diff -u $(EXPECTED_DIR)/case_md5.norm $(TMP_DIR)/case_md5.norm; then \
		echo "PASS: check_case_md5"; \
	else \
		echo "FAIL: check_case_md5"; \
		exit 1; \
	fi

check_case_md5_m: all cases expected
	mkdir -p $(TMP_DIR)
	(cd $(CASE_MD5) && ../../$(BIN) -m) > $(TMP_DIR)/case_md5_m.out
	awk '/^=== /{key=$$4 "|" $$NF; print "G|" key; cur=key; next} NF==0{next} cur!=""{print "P|" cur "|" $$0}' $(TMP_DIR)/case_md5_m.out | LC_ALL=C sort > $(TMP_DIR)/case_md5_m.norm
	@if diff -u $(EXPECTED_DIR)/case_md5_m.norm $(TMP_DIR)/case_md5_m.norm; then \
		echo "PASS: check_case_md5_m"; \
	else \
		echo "FAIL: check_case_md5_m"; \
		exit 1; \
	fi

check_case_md5_dm: all cases expected
	mkdir -p $(TMP_DIR)
	(cd $(CASE_MD5) && ../../$(BIN) -d -m) > $(TMP_DIR)/case_md5_dm.out
	awk '/^=== /{key=$$4 "|" $$NF; print "G|" key; cur=key; next} NF==0{next} cur!=""{print "P|" cur "|" $$0}' $(TMP_DIR)/case_md5_dm.out | LC_ALL=C sort > $(TMP_DIR)/case_md5_dm.norm
	@if diff -u $(EXPECTED_DIR)/case_md5_dm.norm $(TMP_DIR)/case_md5_dm.norm; then \
		echo "PASS: check_case_md5_dm"; \
	else \
		echo "FAIL: check_case_md5_dm"; \
		exit 1; \
	fi

check_cases: check_case_default check_case_date check_case_date_d check_case_md5 check_case_md5_m check_case_md5_dm
	@echo "PASS: check_cases"
