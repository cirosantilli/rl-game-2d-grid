.POSIX:

O ?= 0
G ?= gdb3
STD ?= c++14
CCC ?= g++ -MMD -g$(G) -pedantic-errors -std=$(STD) -O$(O) -Wall -Werror -Wextra
DEP_EXT ?= .d
H_EXT ?= .hpp
PH_EXT ?= .hpp.gch
IN_EXT ?= .cpp
LIBS ?= -lm -lSDL2 -lSDL2_image -lSDL2_ttf
OBJ_EXT ?= .o
OUT_EXT ?= .out
RUN ?= main

INS := $(wildcard *$(IN_EXT))
OBJS := $(INS:$(IN_EXT)=$(OBJ_EXT))
RUN_BASENAME := $(RUN)$(OUT_EXT)

.PHONY: assets-png assets-svg clean run

$(RUN_BASENAME): $(OBJS)
	$(CCC) -o '$@' $+ $(LIBS)

-include $(OBJS:$(OBJ_EXT)=$(DEP_EXT))

%$(OBJ_EXT): %$(IN_EXT)
	$(CCC) -c '$<' -o '$@'

# TODO get precomiled headers working.
#%$(OBJ_EXT): %$(PH_EXT)
#%$(PH_EXT): %$(H_EXT)
#	$(CCC) '$<' -o '$@'

assets-png:
	# Process PNGs downloaded from site further: make white transparent.
	# Also tracking the generated outputs on git: ugly, but I'm afraid people
	# in different OSes won't be able to reproduce my media processing.
	for png in *.orig.png; do \
		convert "$${png}" -threshold 90% -transparent white "$${png%%.*}.png" ;\
	done

assets-svg:
	# One day, https://github.com/game-icons/icons/issues/300 will be solved.
	# and we will track the source SVGs here.
	# For now, just downloading PNGs from website directly, and processing them further.
	for svg in *.svg; do \
		bname="$${svg%.*}" ;\
		inkscape -z -e "$${bname}1.png" -w 512 -h 512 "$$svg" ;\
		convert "$${bname}1.png" -threshold 90% -transparent white "$${bname}.png" ;\
	done

clean:
	rm -f *'$(DEP_EXT)' *'$(OBJ_EXT)' '$(PH_EXT)' '$(RUN_BASENAME)'

run: $(RUN_BASENAME)
	./'$(RUN_BASENAME)'
