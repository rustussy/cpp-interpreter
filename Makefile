CXXFLAGS = -std=c++20 -Wall -Wextra -Wpedantic -ggdb3 -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wcast-align -Wstrict-overflow=5 -Wformat=2 -Wwrite-strings -Wcast-qual -Wswitch-default -Wconversion -DLOCAL -DBOUNDCHK

OUTDIR = ./output
LOGDIR = ./log

CPP = main.cpp
OBJ = $(CPP:%.cpp=$(OUTDIR)/%.o)
BIN = $(OBJ:%.o=%)
DEPS = $(OBJ:%.o=%.d)

ifeq ($(MODE),fast)
CXXFLAGS += -O3 -DNDEBUG -fno-stack-protector -ffast-math -funroll-loops -ftree-vectorize
else
CXXFLAGS += -O0 -g -fsanitize=address,undefined
endif

all: $(BIN)
	$(OUTDIR)/main

-include $(DEPS)

$(OUTDIR)/%.o: %.cpp | $(OUTDIR) ${LOGDIR}
	$(CXX) $(CXXFLAGS) -MMD -c $< -o $@

$(OUTDIR)/% : $(OUTDIR)/%.o
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OUTDIR) $(LOGDIR):
	mkdir -p $@

.PHONY: clean
clean:
	-rm $(OBJ) $(DEPS)
