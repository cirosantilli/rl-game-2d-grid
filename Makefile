.POSIX:

O ?= 0
STD ?= c++14
CCC ?= g++ -ggdb3 -pedantic-errors -std=$(STD) -O$(O) -Wextra
IN_EXT ?= .cpp
LIBS ?= -lm -lSDL2
OUT_EXT ?= .out
RUN ?= main
OBJ_EXT ?= .o

INS := $(wildcard *$(IN_EXT))
OUTS := $(INS:$(IN_EXT)=$(OBJ_EXT))
RUN_BASENAME := $(RUN)$(OUT_EXT)

.PHONY: clean run

$(RUN_BASENAME): $(OUTS)
	$(CCC) -o '$@' $+ $(LIBS)

%$(OBJ_EXT): %$(IN_EXT)
	$(CCC) -c '$<' -o '$@'

clean:
	rm -f *'$(OBJ_EXT)' '$(RUN_BASENAME)'

run: $(RUN_BASENAME)
	./'$(RUN_BASENAME)'
