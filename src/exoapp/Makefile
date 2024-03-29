
# path macros
BIN_PATH := ~/build
ALL_SRC_PATH := ${HOME}/src
OBJ_PATH := obj
SRC_PATH := src
DBG_PATH := debug
RES_PATH := res

# tool macros
CXX := arm-linux-gnueabihf-g++
CXXFLAGS :=   -O3  -lpthread -lrt 
DBGFLAGS := -g 
CCOBJFLAGS := $(CXXFLAGS) \
			 -I${ALL_SRC_PATH}/libs_rerob \
			 -I${ALL_SRC_PATH}/libs_rerob/canlib  \
			 -I${ALL_SRC_PATH}/libs_rerob/utilslib \
			 -I${HOME}/libs/ -I/usr/include/eigen3/ \
			 -c  -lrt  

# compile macros
TARGET_NAME := $(shell basename $(CURDIR))
ifeq ($(OS),Windows_NT)
	TARGET_NAME := $(addsuffix .exe,$(TARGET_NAME))
endif
TARGET := $(BIN_PATH)/$(TARGET_NAME)
TARGET_DEBUG := $(DBG_PATH)/$(TARGET_NAME)

# src files & obj files
SRC := $(foreach x, $(SRC_PATH), $(wildcard $(addprefix $(x)/*,.c*)))
OBJ := $(addprefix $(OBJ_PATH)/, $(addsuffix .o, $(notdir $(basename $(SRC)))))
OBJ_DEBUG := $(addprefix $(DBG_PATH)/, $(addsuffix .o, $(notdir $(basename $(SRC)))))

SRC2 := $(foreach x, ${ALL_SRC_PATH}/libs_rerob/utilslib, $(wildcard $(addprefix $(x)/*,.c*))) 
OBJ +=  $(addsuffix .o, $(basename $(SRC2)) )

SRC2 := $(foreach x, ${HOME}/libs/LowPassFilter, $(wildcard $(addprefix $(x)/*,.c*))) 
OBJ +=  $(addsuffix .o, $(basename $(SRC2)) )

SRC += $(SRC2)

# clean files list
DISTCLEAN_LIST := $(OBJ) \
                  $(OBJ_DEBUG)
CLEAN_LIST := $(TARGET) \
			  $(TARGET_DEBUG) \
			  $(DISTCLEAN_LIST)

# default rule
default: makedir all

run :
	@echo $(SRC)
	@echo $(OBJ)
	./bin/main

send-beaglebone :
	./send_to_beaglebone.sh

# non-phony targets
$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(CXXFLAGS) $(BIN_PATH)/libs/canlib.so
	$(CXX) -o $@.static $(OBJ) $(BIN_PATH)/libs/canlib.a $(CXXFLAGS)

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c*
	$(CXX) $(CCOBJFLAGS) -o $@ $<

$(DBG_PATH)/%.o: $(SRC_PATH)/%.c*
	$(CXX) $(CCOBJFLAGS) $(DBGFLAGS) -o $@ $<

$(TARGET_DEBUG): $(OBJ_DEBUG)
	$(CXX) $(CXXFLAGS) $(DBGFLAGS) $(OBJ_DEBUG) -o $@

# phony rules
.PHONY: makedir
makedir:
	@mkdir -p $(BIN_PATH) $(OBJ_PATH) $(DBG_PATH) $(RES_PATH)

.PHONY: all
all: $(TARGET)

.PHONY: debug
debug: $(TARGET_DEBUG)

.PHONY: clean
clean:
	@echo "SRC"
	@echo $(SRC)
	@echo $(OBJ)
	@echo CLEAN $(CLEAN_LIST)
	@rm -f $(CLEAN_LIST)

.PHONY: distclean
distclean:
	@echo CLEAN $(CLEAN_LIST)
	@rm -f $(DISTCLEAN_LIST)

