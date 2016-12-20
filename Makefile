.POSIX:

O ?= 0
STD ?= c++14
CCC ?= g++ -MMD -ggdb3 -pedantic-errors -std=$(STD) -O$(O) -Wall -Werror -Wextra
DEP_EXT ?= .d
IN_EXT ?= .cpp
LIBS ?= -lm -lSDL2
OBJ_EXT ?= .o
OUT_EXT ?= .out
RUN ?= main

INS := $(wildcard *$(IN_EXT))
OBJS := $(INS:$(IN_EXT)=$(OBJ_EXT))
RUN_BASENAME := $(RUN)$(OUT_EXT)

.PHONY: clean run

$(RUN_BASENAME): $(OBJS)
	$(CCC) -o '$@' $+ $(LIBS)

-include $(OBJS:$(OBJ_EXT)=$(DEP_EXT))

%$(OBJ_EXT): %$(IN_EXT)
	$(CCC) -c '$<' -o '$@'

clean:
	rm -f *'$(DEP_EXT)' *'$(OBJ_EXT)' '$(RUN_BASENAME)'

run: $(RUN_BASENAME)
	./'$(RUN_BASENAME)'
