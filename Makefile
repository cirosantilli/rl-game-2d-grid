.POSIX:

O ?= 0
G ?= gdb3
STD ?= c++14
CCC ?= g++ -MMD -g$(G) -pedantic-errors -std=$(STD) -O$(O) -Wall -Werror -Wextra
DEP_EXT ?= .d
H_EXT ?= .hpp
PH_EXT ?= .hpp.gch
IN_EXT ?= .cpp
LIBS ?= -lm -lSDL2 -lSDL2_ttf
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

# TODO get precomiled headers working.
#%$(OBJ_EXT): %$(PH_EXT)
#%$(PH_EXT): %$(H_EXT)
	#$(CCC) '$<' -o '$@'

clean:
	rm -f *'$(DEP_EXT)' *'$(OBJ_EXT)' '$(PH_EXT)' '$(RUN_BASENAME)'

run: $(RUN_BASENAME)
	./'$(RUN_BASENAME)'
