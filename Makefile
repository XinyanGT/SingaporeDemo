# Dependencies
APP_DIR = app
APP_NAMES = gen viz
APPS = $(patsubst %, $(APP_DIR)/exe/%, $(APP_NAMES))
YANDEX_DIR = yandex
YANDEX_LIB = $(YANDEX_DIR)/lib/libyandex.a
UTILS_DIR = utils
UTILS_LIB = $(UTILS_DIR)/lib/libutils.a
VIZ_DIR = viz
VIZ_LIB = $(VIZ_DIR)/lib/libviz.a
ADIOS_INC = $(shell adios_config -c)
ADIOS_LIB = $(shell adios_config -l)
OPENCV_DIR = /ccs/home/voidp/Software/opencv
OPENCV_LIB = -L$(OPENCV_DIR)/lib -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_contrib

INCS = -I$(YANDEX_DIR)/include -I$(UTILS_DIR)/include -I$(VIZ_DIR)/include $(ADIOS_INC)
LIBS = -L$(YANDEX_DIR)/lib -lyandex -L$(UTILS_DIR)/lib -lutils -L$(VIZ_DIR)/lib -lviz $(ADIOS_LIB) $(OPENCV_LIB)

CC = mpicc

.PHONY: default
default: release

.PHONY: release_all
release_all: clean_all release

.PHONY: debug_all
debug_all: clean_all debug

.PHONY: release
release: CFLAGS += -O3 -g
release: VERSION = release
release: $(APPS)

.PHONY: debug
debug: CFLAGS += -g -O0
debug: VERSION = debug
debug: $(APPS)


.PHONY: debug_all
debug: CFLAGS += -g -O0
debug: VERSION = debug
debug: $(APPS) 


$(APP_DIR)/exe/%: $(APP_DIR)/src/%.c $(UTILS_LIB) $(YANDEX_LIB) $(VIZ_LIB)
	$(CC) $(CFLAGS) $(INCS) $< -o $@ $(LIBS)

$(YANDEX_LIB):
	$(MAKE) $(VERSION) -C $(YANDEX_DIR)

$(UTILS_LIB):
	$(MAKE) $(VERSION) -C $(UTILS_DIR)

$(VIZ_LIB):
	$(MAKE) $(VERSION) -C $(VIZ_DIR)

.PHONY: clean_all
clean_all: clean
	$(MAKE) clean -C $(YANDEX_DIR)
	$(MAKE) clean -C $(UTILS_DIR)
	$(MAKE) clean -C $(VIZ_DIR)


.PHONY: clean
clean: 
	rm -rf a.out


