NAME         := nessemble
BIN_DIR      := /usr/local/bin
RM           := rm -f
CC           := gcc
CC_FLAGS     := -Wall -Wextra
CC_LIB_FLAGS := -lm -lpng
CC_INCLUDES  := /usr/local/include
CC_LIBRARIES := /usr/local/lib
LEX          := flex
LEX_OUT      := lex.yy
LEX_FLAGS    := --outfile=$(LEX_OUT).c
YACC         := bison
YACC_OUT     := y.tab
YACC_FLAGS   := --output=$(YACC_OUT).c --defines --yacc

OPCODES      := opcodes
TEST         := test
UNAME        := $(shell uname -s)

SRCS         := $(YACC_OUT).c $(LEX_OUT).c main.c assemble.c disassemble.c error.c init.c instructions.c list.c macro.c math.c midi.c opcodes.c png.c $(shell ls pseudo/*.c) reference.c simulate.c $(shell ls simulate/*.c) usage.c utils.c wav.c
HDRS         := $(NAME).h init.h license.h
OBJS         := ${SRCS:c=o}

REFERENCE    := reference/ppuctrl.h reference/ppumask.h reference/ppustatus.h reference/oamaddr.h reference/oamdata.h reference/ppuscroll.h reference/ppuaddr.h reference/ppudata.h

ifeq ($(ENV), debug)
	CC_FLAGS += -g
endif

ifeq ($(UNAME), Darwin)
	CC_FLAGS += -ll -I$(CC_INCLUDES) -L$(CC_LIBRARIES) -Qunused-arguments
else
	CC_FLAGS += -lfl -lrt
endif

all: $(NAME)

$(LEX_OUT).c: $(NAME).l
	$(LEX) $(LEX_FLAGS) $<

$(YACC_OUT).c: $(NAME).y
	$(YACC) $(YACC_FLAGS) $<

opcodes.c: opcodes.csv
	./opcodes.sh $< $@

%.o: %.c
	$(CC) -O -c $< $(CC_FLAGS) -o $@

%.h: %.txt
	$(eval STR := _$(shell echo "$@" | awk '{print toupper($$0)}' | sed "s/[^[:alpha:]]/_/g"))
	printf "#ifndef %s\n#define %s\n\n" $(STR) $(STR) > $@
	xxd -i $< >> $@
	printf "\n#endif /* %s */\n" $(STR) >> $@

reference.c: ${REFERENCE:txt=h} reference.h

init.c: init.h

init.h: ${init.txt:txt=h}

usage.c: license.h

license.h: ${licence.txt:txt=h}

$(NAME): $(OBJS) $(HDRS)
	$(CC) -o $@ $(OBJS) $(CC_FLAGS) $(CC_LIB_FLAGS)

$(TEST): all
	@./$(TEST).sh

check: all
	@./check.sh

install: all
	strip $(NAME)
	install -m 0755 $(NAME) $(BIN_DIR)

uninstall:
	rm -f $(BIN_DIR)/$(NAME)

.PHONY: clean
clean:
	$(RM) $(NAME) $(YACC_OUT).c $(YACC_OUT).h $(LEX_OUT).c $(OPCODES).c $(OBJS) init.h license.h reference/*.h check/suite_*
