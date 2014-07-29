YANDEX_DIR = yandex
YANDEX_LIB = $(YANDEX_DIR)/lib/yandex.a
UTILS_DIR = utils
UTILS_LIB = $(UTILS_DIR)/lib/utils.a
ADIOS_INC = $(shell adios_config -c)
ADIOS_LINK = $(shell adios_config -l)
CC = mpicc
APP = Jul-25.c

.PHONY: default
default: release

.PHONY: release_all
release_all: clean_all release

.PHONY: debug_all
debug_all: clean_all debug

.PHONY: release
release: CFLAGS += -O3
release: VERSION = release
release: a.out

.PHONY: debug
debug: CFLAGS += -g -O0
debug: VERSION = debug
debug: a.out


.PHONY: debug_all
debug: CFLAGS += -g -O0
debug: VERSION = debug
debug: a.out 


a.out: $(APP) $(UTILS_LIB) $(YANDEX_LIB)
	$(CC) $(CFLAGS) $(ADIOS_INC) -I$(YANDEX_DIR)/include -I$(UTILS_DIR)/include $(APP) -L$(YANDEX_DIR)/lib -L$(UTILS_DIR)/lib -lyandex -lutils $(ADIOS_LINK)

$(YANDEX_LIB):
	$(MAKE) $(VERSION) -C $(YANDEX_DIR)

$(UTILS_LIB):
	$(MAKE) $(VERSION) -C $(UTILS_DIR)

.PHONY: clean_all
clean_all: clean
	$(MAKE) clean -C $(YANDEX_DIR)
	$(MAKE) clean -C $(UTILS_DIR)


.PHONY: clean
clean: 
	rm -rf a.out


