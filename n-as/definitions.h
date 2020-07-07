#ifndef DEFINITIONS
#define DEFINITIONS

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>
#include <limits.h>
#include <float.h>

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>

void yyerror(const char*);

struct section {
	const char* name; // pointer into the assembly string
	uint32_t offset; // offset from the start of the binary. one section is selected as the start of the binary, and all others are relative to it
	void* data; // the contents of the section
	uint32_t size; // size of the section at this time
	uint32_t nextindex; // if (data+nextindex) > size and something has to be put in the section, the section is made bigger
};


struct fixup {
	const char* name; // pointer into the assembly string
	uint32_t offset; // offset from the beginning of the section
	uint8_t maxbits; // maximum number of bits for this fixup. if more are needed, an error is generated
	uint8_t section; // index into the section table
};


struct label {
	uint32_t offset; // offset from the beginning of the section
	const char* name; // pointer into the assembly string
	uint8_t section; // index into the section table
};


enum condition {
	EQ=0,NE,CSHS,CCLO,MI,PL,VS,VC,HI,LS,GE,LT,GT,LE,ALWAYS,NEVER
};
enum condition get_condition(char* str);

bool add_section(struct section *sect);
bool add_fixup(struct fixup *fix);
bool add_label(struct label *l);

bool section_write(int sect,void* data,uint32_t size,int offset);



int find_section(const char *name);
// returns a pointer to the label with the name, or NULL if not found
struct label* find_label(const char *name);

extern int current_section;
extern struct section** sections;

#endif