.PHONY: all tests clean_out init all_tests


TESTS = test_cli_json

.PHONY: $(TESTS)

make = make --no-print-dir


ifeq ($(V),)
Q:=@
endif

grep_results := grep -H -e IGNORE -e FAIL -e Aborted 

all_tests:
	$(Q) $(make) all C_WARN='-w'

all: clean_out init
	@echo "\n### Making and running tests: " $(TESTS) "\n"
	$(Q) $(make) -B tests
	
$(TESTS):
	$(Q) $(make) -B $(addsuffix .log,$(addprefix build/,$@))

clean_out:
	$(Q) -rm -f build/test_*.out
	$(Q) -rm -f test_env/*
init:
	$(Q) mkdir -p build
	$(Q) mkdir -p test_env

	
tests = $(addsuffix .log,$(addprefix build/,$(TESTS)))
tests: $(tests)
	$(Q) $(grep_results) build/*.log  || true

SRC_BASE=../..
COMP_DIR=src/components
COMPSHARE_DIR=components-mcu/components
COMPSHARE_PATH := $(SRC_BASE)/$(COMPSHARE_DIR)
COMP_PATH := $(SRC_BASE)/$(COMP_DIR)
EXTRA_INCDIR = test_runner $(SRC_BASE)/src/main ./unity/src
C_WARN ?= -Wall -Wno-unused-result

INCLUDE := $(addprefix -I,$(EXTRA_INCDIR))

build/test_%.out:
	$(Q) gcc  -g3 $(C_WARN) -flto -O0 -o $@  $^ unity/src/unity.c test_runner/test_runner.c $(setup) $(INCLUDE) -DHOST_TESTING -DTEST_HOST -DTEST -lm #-H


%.log:%.out
	$(Q) mkdir -p test_env
	$(Q) cd ./test_env && echo "** Run test: $<\n" && ../$< >../$@ 2>&1 || ($(grep_results) ../$@ && false)
	$(Q) $(grep_results) $@ || true


setup=test_runner/setup.c
################################
build/test_cli_json.out: src/test_cli_json.c $(SRC_BASE)/src/main/cli/cli_json.c $(SRC_BASE)/src/main/json/json.c
