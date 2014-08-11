MODULES = reader writer retriever stimer upsampler decomp
UNITTESTS = reader writer retriever stimer upsampler decomp
LIBNAME = utils

SDIR = src
IDIR = include
ODIR = obj
LDIR = lib
EDIR = exe
TDIR = unit_test
BUILD = build

TARGET = $(LDIR)/lib$(LIBNAME).a    # library that will be created
CC = mpicc
CFLAGS = -I$(IDIR) -Wall

OBJECTS = $(patsubst %, $(ODIR)/%.o, $(MODULES))
HEADERS = $(patsubst %, $(IDIR)/%.h, $(MODULES))
TESTSRC = $(patsubst %, $(TDIR)/test_%.c, $(UNITTESTS))
TESTS = $(patsubst %, $(EDIR)/test_%, $(UNITTESTS))

.PHONY: release
release: CFLAGS += -O3 -g
release: $(TARGET) $(TESTS)

.PHONY: debug
debug: CFLAGS += -g -O0
debug: $(TARGET) $(TESTS)

.PHONY: clean
clean:
	rm -f $(OBJECTS) $(TARGET)
	rm -rf $(TESTS)

$(TARGET): $(OBJECTS)
	@mkdir -p $(LDIR)
	ar rcs $@ $^

$(ODIR)/%.o: $(SDIR)/%.c $(HEADERS)
	@mkdir -p $(ODIR)
	$(CC) -c $(CFLAGS) `adios_config -c` $< -o $@

$(EDIR)/%: $(TDIR)/%.c $(HEADERS) $(TARGET)
	@mkdir -p $(EDIR)
	$(CC) $(CFLAGS) `adios_config -c` $< -o $@ -L$(LDIR) -l$(LIBNAME) `adios_config -l`


