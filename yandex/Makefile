MODULES = quantizer buckets yandex hquantizer
UNITTESTS = quantizer buckets yandex hquantizer
LIBNAME = yandex
UTILS_DIR = ../utils

SDIR = src
IDIR = include
ODIR = obj
LDIR = lib
EDIR = exe
TDIR = unit_test
BUILD = build

TARGET = $(LDIR)/lib$(LIBNAME).a    # library that will be created
CC = mpicc
CFLAGS = -I$(IDIR) -I$(UTILS_DIR)/include

OBJECTS = $(patsubst %, $(ODIR)/%.o, $(MODULES))
HEADERS = $(patsubst %, $(IDIR)/%.h, $(MODULES))
TESTSRC = $(patsubst %, $(TDIR)/test_%.c, $(UNITTESTS))
TESTS = $(patsubst %, $(EDIR)/test_%, $(UNITTESTS))

.PHONY: release
release: CFLAGS += -O3
release: $(TARGET) $(TESTS)

.PHONY: debug
debug: CFLAGS += -g -O0
debug: $(TARGET) $(TESTS)

.PHONY: clean
clean:
	rm -f $(OBJECTS) $(TARGET)
	rm -f $(TESTS)

$(TARGET): $(OBJECTS)
	@mkdir -p $(LDIR)
	ar rcs $@ $^

$(ODIR)/%.o: $(SDIR)/%.c $(HEADERS)
	@mkdir -p $(ODIR)
	$(CC) -c $(CFLAGS) $< -o $@

$(EDIR)/%: $(TDIR)/%.c $(HEADERS) $(TARGET)
	@mkdir -p $(EDIR)
	$(CC) $(CFLAGS) $< -o $@ -L$(LDIR) -l$(LIBNAME) -L$(UTILS_DIR)/lib -lutils


