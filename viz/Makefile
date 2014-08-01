MODULES = viz
UNITTESTS = viz
LIBNAME = viz

SDIR = src
IDIR = include
ODIR = obj
LDIR = lib
EDIR = exe
TDIR = unit_test
BUILD = build

OBJECTS = $(patsubst %, $(ODIR)/%.o, $(MODULES))
HEADERS = $(patsubst %, $(IDIR)/%.h, $(MODULES))
TESTSRC = $(patsubst %, $(TDIR)/test_%.c, $(UNITTESTS))
TESTS = $(patsubst %, $(EDIR)/test_%, $(UNITTESTS))

TARGET = $(LDIR)/lib$(LIBNAME).a    # library that will be created
CFLAGS = -I$(IDIR)
CPP = g++
CC = gcc

OPENCV_DIR = /ccs/home/voidp/Software/opencv
OPENCV_INC = -I$(OPENCV_DIR)/include 
OPENCV_LIB = -L$(OPENCV_DIR)/lib -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_contrib

INCS = $(OPENCV_INC)
LIBS = $(OPENCV_LIB)


.PHONY: release
release: CFLAGS += -O3
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

$(ODIR)/%.o: $(SDIR)/%.cpp $(HEADERS)
	@mkdir -p $(ODIR)
	$(CPP) -c $(CFLAGS) $(INCS) $< -o $@

$(EDIR)/%: $(TDIR)/%.c $(HEADERS) $(TARGET)
	@mkdir -p $(EDIR)
	$(CC) $(CFLAGS) $< -o $@ -L$(LDIR) -l$(LIBNAME) $(LIBS)

