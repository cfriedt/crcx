# MIT License
#
# Copyright (c) 2020 Christopher Friedt
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

.PHONY: all clean check gtest gcov clangtidy doc format

TAG = crcx
TOOLCHAIN=gcc
QUIET=@
DEBUG=1

ifeq (gcc,$(TOOLCHAIN))
CROSS_COMPILE ?=
CC  := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
AR  := $(CROSS_COMPILE)ar
endif
ifeq (clang,$(TOOLCHAIN))
CC  := clang
CXX := clang++
AR  := ar
endif

ARFLAGS :=
CFLAGS :=
CPPFLAGS :=
CXXFLAGS :=
LDFLAGS :=

GCOVFLAGS :=
GCOVRFLAGS :=
CTIDYFLAGS :=

ARFLAGS += crs
LDFLAGS += -Wl,-rpath,/usr/local/lib
CFLAGS += -Wall -Wextra -Werror
#CFLAGS += -Wpedantic
##pedantic, but not *that* pedantic
#CFLAGS += -Wno-variadic-macros -Wno-gnu-zero-variadic-macro-arguments
##pedantic, but not *that* pedantic
#CFLAGS += -Wno-gnu-statement-expression
CFLAGS += -mtune=native
CFLAGS += -fPIC
#ifeq ($(DEBUG),)
#CFLAGS += -O3 -fomit-frame-pointer -DNDEBUG
#else
CFLAGS += -O0 -g -DDEBUG
#endif

# gcov
CFLAGS += -fprofile-arcs -ftest-coverage
GCOVFLAGS += -r
GCOVRFLAGS += -r $(shell pwd) -e '.*-test.cpp' -e 'util/'
LDFLAGS += --coverage

CXXFLAGS += $(CFLAGS)

CFLAGS += -std=c11
CXXFLAGS += -std=c++14

ifeq (gcc,$(TOOLCHAIN))
CXXFLAGS += -D_GLIBCXX_USE_CXX11_ABI=1
endif

ifeq (clang,$(TOOLCHAIN))
CXXFLAGS += -stdlib=libc++
endif

CTIDYCHECKS :=
CTIDYCHECKS += clang-analyzer-core*
CTIDYCHECKS += clang-analyzer-security*
CTIDYCHECKS += clang-analyzer-unix*
CTIDYCHECKS += clang-analyzer-valist*
CTIDYCHECKS += clang-analyzer-optin.cplusplus*
CTIDYCHECKS += clang-analyzer-optin.portability*
CTIDYCHECKS += clang-analyzer-nullability*
CTIDYCHECKS += clang-analyzer-deadcode*
CTIDYCHECKS += clang-analyzer-cplusplus*
NOTHING :=
SPACE := $(NOTHING) $(NOTHING)
COMMA := ,
CTIDYCHECKLIST := $(subst $(SPACE),$(COMMA),$(strip $(CTIDYCHECKS)))

CTIDYFLAGS += -header-filter='.*'
CTIDYFLAGS += -checks='$(CTIDYCHECKLIST)'
CTIDYFLAGS += -warnings-as-errors='$(CTIDYCHECKLIST)'


GTEST_CFLAGS = $(shell pkg-config --cflags gtest gtest_main)
GTEST_LIBS = $(shell pkg-config --libs gtest gtest_main)

CLIB := lib$(TAG).a
SOCLIB := lib$(TAG).so
SOCXXLIB := lib$(TAG)xx.so
LIB = $(CLIB) $(SOCLIB) $(CXXLIB) $(SOCXXLIB)
EXE :=

CPPFLAGS += -Isrc

CSRC :=
CHDR :=
CXXSRC :=
HDR :=
PYSRC :=
CXXEXE :=
PYEXE :=
CEXE :=
TEST :=

OBJ :=

BUILT_SOURCES :=
CLEANFILES :=

CSRC += $(shell find * -name '*.c')
CHDR += $(shell find * -name '*.h')
CXXSRC += $(shell find * -name '*.cpp')
CXXSRC := $(filter-out %-test.cpp,$(CXXSRC))
TEST += $(shell find * -name '*-test.cpp')

TESTEXE := $(TEST:.cpp=)

EXE += $(CXXEXE) $(CEXE) $(PYEXE) $(TESTEXE)

all: $(LIB) $(EXE)

CXXSRC := $(filter-out $(addsuffix .cpp,$(CXXEXE)),$(CXXSRC))
CXXOBJ = $(CXXSRC:.cpp=.o)
OBJ += $(CXXOBJ)

CSRC := $(filter-out $(addsuffix .c,$(CEXE)),$(CSRC))
COBJ := $(CSRC:.c=.o)
OBJ += $(COBJ)

$(CLIB): $(COBJ)
	@echo "[AR] $@"
	$(QUIET)$(AR) $(ARFLAGS) $@ $^

$(SOCLIB): $(CLIB)
	@echo "[CCLD] $@"
	$(QUIET)$(CC) -shared -Wl,-soname,$@ -Wl,-no-undefined $(LDFLAGS) -o $@ -Wl,--whole-archive $< -Wl,--no-whole-archive

$(CXXLIB): $(CXXOBJ)
	@echo "[AR] $@"
	$(QUIET)$(AR) $(ARFLAGS) $@ $^

$(SOCXXLIB): $(CXXLIB) $(SOCLIB)
	@echo "[CXXLD] $@"
	$(QUIET)$(CXX) -shared -Wl,-soname,$@ -Wl,-no-undefined $(LDFLAGS) -o $@ -Wl,--whole-archive $< -Wl,--no-whole-archive $(SOCLIB)

%-test: %-test.cpp $(CXXLIB) $(CLIB)
	@echo "[CXXLD] $@"
	$(QUIET)$(CXX) $(CPPFLAGS) -I$(GENCXXDIR) $(CXXFLAGS) $(GTEST_CXXFLAGS) $(LDFLAGS) $< -o $@ $(CLIB) $(CXXLIB) $(THRIFT_CXX_LIBS) $(GTEST_LIBS)

%.o: %.cpp $(HDR)
	@echo "[CXX] $@"
	$(QUIET)$(CXX) $(CPPFLAGS) -I$(GENCXXDIR) $(CXXFLAGS) -c $< -o $@

%.o: %.c $(HDR)
	@echo "[CC] $@"
	$(QUIET)$(CC) $(CPPFLAGS) -I$(GENCDIR) $(CFLAGS) -c $< -o $@

clean:
	@echo "[RM] LIB"
	$(QUIET)rm -Rf $(LIB)
	@echo "[RM] EXE"
	$(QUIET)rm -Rf $(EXE)
	@echo "[RM] OBJ"
	$(QUIET)rm -Rf $(OBJ)
	@echo "[RM] GEN"
	$(QUIET)rm -Rf $(shell find * -name '*.gcna' -o -name '*.gcno' -o -name '*.gcda' -o -name '*.clangtidy')
	@echo "[RM] DOC"
	$(QUIET)rm -Rf html/ latex/

check:
	$(MAKE) gtest
	$(MAKE) gcov
	$(MAKE) clangtidy

gtest: $(TESTEXE)
	NTEST=0; \
	NPASS=0; \
	if [ -z "$(strip $(TESTEXE))" ]; then \
			exit 0; \
	fi; \
	for i in $(EXE); do \
			./$$i; \
			if [ $$? -eq 0 ]; then \
					NPASS=$$((NPASS+1)); \
			fi; \
			NTEST=$$((NTEST+1)); \
	done; \
	if [ $$NPASS -eq $$NTEST ]; then \
			exit 0; \
	else \
			exit 1; \
	fi

gcov: $(TESTEXE)
	$(QUIET)if [ -z "$(strip $(TESTEXE))" ]; then \
			exit 0; \
	fi; \
	for i in $(patsubst %-test,,$(TESTEXE)); do \
			echo "[GCOV] $${i}.c"; \
			gcov $(GCOVFLAGS) $${i}.c &> /dev/null; \
	done; \
	gcovr $(GCOVRFLAGS); \
	PCNT=`gcovr $(GCOVRFLAGS) | grep "^TOTAL" | tail -n 1 | awk '{print $$4}' | sed -e 's|%||'`; \
	if [ $${PCNT} -lt 90 ]; then \
			exit 1; \
	fi

%.clangtidy: %.c
	clang-tidy $(CTIDYFLAGS) $< -- $(CPPFLAGS) $(CFLAGS) &> $@ || cat $@

%.clangtidy: %.cpp
	clang-tidy $(CTIDYFLAGS) $< -- $(CPPFLAGS) $(CXXFLAGS) &> $@ || cat $@

clangtidy: $(addsuffix .clangtidy,$(CSRC:.c=) $(CXXSRC:.cpp=))

doc:
	doxygen Doxyfile

format:
	clang-format -i $(CSRC) $(CHDR) $(shell find * -name '*.cpp')
