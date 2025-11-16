# Check for OS - Linux or Darwin (macOS)
KERNEL := $(shell uname)
MACHINE := $(shell uname -m)
KERNEL_VER := $(shell uname -v)
GIT_BRANCH := $(shell git rev-parse --abbrev-ref HEAD)

# Check for availability of MPI
MPIVER := $(shell [ ! -z ${EBVERSIONOPENMPI} ] && echo true || echo false)

CFLAGS = -std=gnu99 -Wall -Wno-unknown-pragmas -Wno-deprecated-declarations -Os -msse2 -mavx -I /usr/local/include
CFLAGS += -DDEBUG

LDFLAGS = -L lib -L /usr/local/lib -lrs

OBJS = log.o les.o adm.o rcs.o obj.o pos.o rs.o
OBJS_PATH = obj
OBJS_WITH_PATH = $(addprefix $(OBJS_PATH)/, $(OBJS))

MYLIB = lib/librs.a

PROGS = simradar
PROGS += simple_ppi simple_dbs lsiq 
PROGS += cldemo test_clreduce test_make_pulse
PROGS += rsutil

MPI_PROGS =

# The command echo from macOS and Ubuntu needs no -e
ECHO_FLAG = -e
ifneq (, $(findstring Darwin, $(KERNEL_VER)))
	ECHO_FLAG =
endif
ifneq (, $(findstring Ubuntu, $(KERNEL_VER)))
	ECHO_FLAG =
endif

ifeq ($(KERNEL), Darwin)
	# macOS
	CC = clang
	CFLAGS += -D_DARWIN_C_SOURCE
	LDFLAGS += -framework OpenCL
else
	# Linux systems, mainly schooner of OSCER
	CC = gcc
	CFLAGS += -D_GNU_SOURCE
	CFLAGS += -I /opt/oscer/software/CUDA/8.0.44-GCC-4.9.3-2.25/include
	CFLAGS += -I /opt/oscer/software/OpenMPI/1.10.2-GCC-4.9.3-2.25/include
	LDFLAGS += -L /opt/oscer/software/CUDA/8.0.44-GCC-4.9.3-2.25/lib64 -lOpenCL
	ifeq ($(MPIVER), true)
		# Cluster with OpenMPI support
		MPI_PROGS += simradar-mpi
		MPI_CFLAGS = -I /opt/oscer/software/OpenMPI/1.10.2-GCC-4.9.3-2.25/include
		MPI_LDFLAGS = -L /opt/oscer/software/OpenMPI/1.10.2-GCC-4.9.3-2.25/lib -lmpi
	endif
endif

LDFLAGS += -lm -lpthread

# Test programs
TEST_PROGS = tests/test_string_safety tests/test_pos_parsing tests/test_rs_tables tests/test_data_loaders tests/test_rs_integration tests/test_data_integration
TEST_CFLAGS = $(CFLAGS) -I.

# Coverage flags
COV_CFLAGS = $(CFLAGS) -I. --coverage -O0 -g
COV_LDFLAGS = --coverage
COV_DIR = coverage
COV_TEST_PROGS = $(addprefix $(COV_DIR)/, $(notdir $(TEST_PROGS)))
COV_OBJS = $(addprefix $(COV_DIR)/, $(OBJS))

all: $(MYLIB) $(PROGS) $(MPI_PROGS)

showinfo:
	@echo $(ECHO_FLAG) "KERNEL_VER = \033[38;5;15m$(KERNEL_VER)\033[0m"
	@echo $(ECHO_FLAG) "KERNEL = \033[38;5;15m$(KERNEL)\033[0m"
	@echo $(ECHO_FLAG) "MACHINE = \033[38;5;220m$(MACHINE)\033[0m"
	@echo $(ECHO_FLAG) "GIT_BRANCH = \033[38;5;46m$(GIT_BRANCH)\033[0m"

#$(OBJS): %.o: %.c %.h rs_types.h
#	$(CC) $(CFLAGS) -c $< -o $@

$(OBJS_PATH)/%.o: %.c | $(OBJS_PATH)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJS_PATH):
	mkdir -p $@

lib/librs.a: $(OBJS_WITH_PATH)
	mkdir -p lib
	ar rvcs $@ $(OBJS_WITH_PATH)

$(PROGS): %: %.c $(MYLIB)
ifeq ($(KERNEL), Darwin)
	@echo "\033[38;5;203m$@\033[0m"
else
	@echo $(ECHO_FLAG) "\033[38;5;203m$@\033[0m"
endif
	$(CC) $(CFLAGS) -o $@ $@.c $(LDFLAGS)

$(MPI_PROGS): %: %.c $(MYLIB)
	$(CC) $(CFLAGS) -D_OPEN_MPI $(MPI_CFLAGS) -o $@ $@.c $(LDFLAGS) $(MPI_LDFLAGS)

prep: simradar-mpi.c
	@ln -sfn simradar.c simradar-mpi.c

clean:
	rm -f $(OBJS_PATH)/*.o *.a
	rm -f $(MYLIB) $(PROGS) $(MPI_PROGS)
	rm -f $(TEST_PROGS)
	rm -rf *.dSYM

# Test targets
test: $(TEST_PROGS)
	@echo $(ECHO_FLAG) "\n\033[38;5;46m=== Running Unit Tests ===\033[0m\n"
	@for test in $(TEST_PROGS); do \
		echo $(ECHO_FLAG) "\033[38;5;220mRunning $$test\033[0m"; \
		./$$test || exit 1; \
		echo ""; \
	done
	@echo $(ECHO_FLAG) "\033[38;5;46m=== All Tests Passed ===\033[0m\n"

tests/test_string_safety: tests/test_string_safety.c
	@mkdir -p tests
	$(CC) $(TEST_CFLAGS) -o $@ $<

tests/test_pos_parsing: tests/test_pos_parsing.c $(MYLIB)
	@mkdir -p tests
	$(CC) $(TEST_CFLAGS) -o $@ $< $(LDFLAGS)

tests/test_rs_tables: tests/test_rs_tables.c
	@mkdir -p tests
	$(CC) $(TEST_CFLAGS) -o $@ $< -lm

tests/test_data_loaders: tests/test_data_loaders.c
	@mkdir -p tests
	$(CC) $(TEST_CFLAGS) -o $@ $< -lm

tests/test_rs_integration: tests/test_rs_integration.c $(MYLIB)
	@mkdir -p tests
	$(CC) $(TEST_CFLAGS) -o $@ $< $(LDFLAGS)

tests/test_data_integration: tests/test_data_integration.c $(MYLIB)
	@mkdir -p tests
	$(CC) $(TEST_CFLAGS) -o $@ $< $(LDFLAGS)

# Coverage targets
coverage: coverage-build coverage-run coverage-report

coverage-build: $(COV_DIR)/librs.a $(COV_TEST_PROGS)

$(COV_DIR)/librs.a: $(COV_OBJS)
	@mkdir -p $(COV_DIR) lib
	ar rcs $@ $^

$(COV_DIR)/%.o: %.c
	@mkdir -p $(COV_DIR)
	$(CC) $(COV_CFLAGS) -c -o $@ $<

$(COV_DIR)/test_string_safety: tests/test_string_safety.c
	@mkdir -p $(COV_DIR)
	$(CC) $(COV_CFLAGS) -o $@ $< $(COV_LDFLAGS)

$(COV_DIR)/test_rs_tables: tests/test_rs_tables.c
	@mkdir -p $(COV_DIR)
	$(CC) $(COV_CFLAGS) -o $@ $< -lm $(COV_LDFLAGS)

$(COV_DIR)/test_data_loaders: tests/test_data_loaders.c
	@mkdir -p $(COV_DIR)
	$(CC) $(COV_CFLAGS) -o $@ $< -lm $(COV_LDFLAGS)

$(COV_DIR)/test_pos_parsing: tests/test_pos_parsing.c $(COV_DIR)/librs.a
	@mkdir -p $(COV_DIR)
	$(CC) $(COV_CFLAGS) -o $@ $< -L $(COV_DIR) -lrs -lm -lpthread $(COV_LDFLAGS)

coverage-run: coverage-build
	@echo $(ECHO_FLAG) "\n\033[38;5;46m=== Running Tests with Coverage ===\033[0m\n"
	@for test in $(COV_TEST_PROGS); do \
		echo $(ECHO_FLAG) "\033[38;5;220mRunning $$test\033[0m"; \
		./$$test > /dev/null 2>&1 || true; \
	done
	@echo $(ECHO_FLAG) "\033[38;5;46m=== Coverage Data Collected ===\033[0m\n"

coverage-report:
	@echo $(ECHO_FLAG) "\033[38;5;46m=== Generating Coverage Report ===\033[0m\n"
	@which lcov > /dev/null 2>&1 && \
		lcov --capture --directory $(COV_DIR) --directory . --output-file $(COV_DIR)/coverage.info --quiet && \
		lcov --remove $(COV_DIR)/coverage.info '/usr/*' '*/tests/*' --output-file $(COV_DIR)/coverage_filtered.info --quiet && \
		genhtml $(COV_DIR)/coverage_filtered.info --output-directory $(COV_DIR)/html --quiet && \
		echo $(ECHO_FLAG) "\033[38;5;46mCoverage report generated: $(COV_DIR)/html/index.html\033[0m\n" && \
		lcov --summary $(COV_DIR)/coverage_filtered.info || \
		echo $(ECHO_FLAG) "\033[38;5;208mWarning: lcov not installed. Install with: sudo apt-get install lcov\033[0m\n"

coverage-clean:
	rm -rf $(COV_DIR)
	find . -name "*.gcda" -delete
	find . -name "*.gcno" -delete
	find . -name "*.gcov" -delete

.PHONY: test clean showinfo prep coverage coverage-build coverage-run coverage-report coverage-clean
