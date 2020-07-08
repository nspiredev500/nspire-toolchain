DEBUG = FALSE

GCC = nspire-gcc
AS  = nspire-as
GXX = nspire-g++
LD  = nspire-ld
GENZEHN = genzehn

GCCFLAGS = -Wall -W -marm -Os
LDFLAGS =
ZEHNFLAGS = --name "n-as" --240x320-support true

OBJS = $(patsubst %.c, %.o, $(shell find . -name \*.c))
OBJS += $(patsubst %.cpp, %.o, $(shell find . -name \*.cpp))
OBJS += $(patsubst %.S, %.o, $(shell find . -name \*.S))
EXE = n-as
DISTDIR = .
vpath %.tns $(DISTDIR)
vpath %.elf $(DISTDIR)

all: lexer

test: lexer
	cat asm.txt | ./lexer a a
	arm-none-eabi-objdump sectiondump -D -b binary -marm -z


lex: lexer

# -Wl,-wrap=malloc -Wl,-wrap=free

lexer: lex.yy.c parser.tab.c 
	gcc -Os lex.yy.c parser.tab.c -o lexer


lex.yy.c: lexer.l main.c definitions.h
	flex lexer.l


parser.tab.c: parser.y logic.c definitions.h
	bison -d parser.y



%.o: %.c
	$(GCC) $(GCCFLAGS) -c $< -o $@

%.o: %.cpp
	$(GXX) $(GCCFLAGS) -c $< -o $@
	
%.o: %.S
	$(AS) -c $< -o $@

$(EXE).elf: $(OBJS)
	mkdir -p $(DISTDIR)
	$(LD) $^ -o $@ $(LDFLAGS)

$(EXE).tns: $(EXE).elf
	$(GENZEHN) --input $^ --output $@.zehn $(ZEHNFLAGS)
	make-prg $@.zehn $@
	rm $@.zehn

clean:
	-rm -f $(OBJS) $(DISTDIR)/$(EXE).tns $(DISTDIR)/$(EXE).elf $(DISTDIR)/$(EXE).zehn lex.yy.c lexer parser.tab.c parser.tab.h
