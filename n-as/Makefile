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


calc: lex.yy.c parser.tab.c 
	$(GCC) -Wall -W -marm -Wno-unused-parameter -Wno-switch -Wno-unused-function -Os lex.yy.c parser.tab.c -o lexer -Wl,-wrap=malloc -Wl,-wrap=free
	$(GENZEHN) --input lexer --output lexer.tns $(ZEHNFLAGS)


# -Mforce-thumb
test: lexer
	cat asm.txt | ./lexer a a
	arm-none-eabi-objdump sectiondump -D -b binary -marm -z

test_thumb: lexer
	cat asm.txt | ./lexer a a
	arm-none-eabi-objdump sectiondump -D -b binary -marm -z -Mforce-thumb


lex: lexer

# IMPORTANT: the malloc wrapper doesn't catch the malloc callst made from inside libc, so strduped memory isn't counted
# -Wl,-wrap=malloc -Wl,-wrap=free
# -static-libgcc --static
lexer: lex.yy.c parser.tab.c 
	gcc -Os lex.yy.c parser.tab.c -o lexer -Wl,-wrap=malloc -Wl,-wrap=free


lex.yy.c: lexer.l main.c definitions.h assembler.h
	flex lexer.l


parser.tab.c: parser.y logic.c definitions.h assembler.h
	bison -d parser.y


clean:
	-rm -f $(OBJS) $(DISTDIR)/$(EXE).tns $(DISTDIR)/$(EXE).elf $(DISTDIR)/$(EXE).zehn lex.yy.c lexer parser.tab.c parser.tab.h lexer.tns
