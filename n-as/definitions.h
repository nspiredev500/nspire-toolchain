#ifndef DEFINITIONS
#define DEFINITIONS

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>
#include <limits.h>
#include <float.h>
#include <errno.h>

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>

void yyerror(const char*);

struct memory_instruction {
	uint32_t test;
};


struct section {
	char* name; // must be freed
	uint32_t offset; // offset from the start of the binary. one section is selected as the start of the binary, and all others are relative to it
	void* data; // the contents of the section
	uint32_t size; // size of the section at this time
	uint32_t nextindex; // if (data+nextindex) > size and something has to be put in the section, the section is made bigger
};


struct fixup {
	char* name; // must be freed
	uint32_t offset; // offset from the beginning of the section
	uint8_t maxbits; // maximum number of bits for this fixup. if more are needed, an error is generated
	uint8_t section; // index into the section table
};


struct label {
	uint32_t offset; // offset from the beginning of the section
	char* name; // must be freed
	uint8_t section; // index into the section table
};


enum condition {
	EQ=0,NE,CSHS,CCLO,MI,PL,VS,VC,HI,LS,GE,LT,GT,LE,ALWAYS,NEVER // IMPORTANT: NEVER is not allowed in arm9, the behavior is UNPREDICTABLE 
};
enum condition get_condition(char* str,int);

bool add_section(struct section *sect);
bool add_fixup(struct fixup *fix);
bool add_label(struct label *l);

bool section_write(int sect,void* data,uint32_t size,int offset);

// frees all labels, fixups and sections
void free_data();

enum addressing_mode {
	pre_indexed_addressing_mode, post_indexed_addressing_mode,offset_addressing_mode
};

#include "assembler.h"

extern uint16_t assembler_flags;

uint16_t register_range(int64_t r1, int64_t r2);

int64_t string_to_immediate(char* str,int base);





void assemble_mul(uint8_t inst,uint8_t mul_width,uint8_t update_flags, uint8_t flags,int64_t reg1,int64_t reg2,int64_t reg3,int64_t reg4);

void assemble_swp(uint8_t b,uint8_t flags,int64_t reg1,int64_t reg2,int64_t reg3);

void assemble_coproc(uint8_t mrc,uint8_t flags,int64_t coproc,int64_t opcode1,int64_t reg,int64_t coproc_reg1,int64_t coproc_reg2,int64_t opcode2);

void assemble_swi(uint8_t flags, int64_t imm);


void assemble_clz(uint8_t flags,int64_t reg1, int64_t reg2);


void assemble_mem_multiple(uint8_t l,uint8_t adr_mode,uint8_t flags, int64_t reg1,uint8_t update_reg,uint16_t reglist,uint8_t user_mode_regs);

void assemble_branch(uint8_t l,uint8_t flags,int64_t imm);


void assemble_mem_half_signed_imm(uint8_t l,uint8_t width,uint8_t flags, int64_t reg1, int64_t reg2, int64_t imm,enum addressing_mode addressing,uint8_t update_reg);
void assemble_mem_half_signed_reg_offset(uint8_t l,uint8_t width,uint8_t flags, int64_t reg1, int64_t reg2, int64_t reg3,enum addressing_mode addressing,uint8_t update_reg,uint8_t u);


void assemble_mem_word_ubyte_imm(uint8_t l,uint8_t b, uint8_t t,uint8_t flags, int64_t reg1, int64_t reg2, int64_t imm,enum addressing_mode addressing,uint8_t update_reg);
void assemble_mem_word_ubyte_reg_offset(uint8_t l,uint8_t b, uint8_t t,uint8_t flags, int64_t reg1, int64_t reg2, int64_t reg3,enum addressing_mode addressing,uint8_t update_reg,uint8_t u);
void assemble_mem_word_ubyte_reg_offset_scaled(uint8_t l,uint8_t b, uint8_t t,uint8_t flags, int64_t reg1, int64_t reg2, int64_t reg3,uint8_t shift_type, int64_t shift_val,enum addressing_mode addressing,uint8_t update_reg,uint8_t u);



void assemble_comp_reg_imm(uint32_t opcode,uint8_t flags,int64_t reg,int64_t imm);
void assemble_comp_reg_reg(uint32_t opcode,uint8_t flags,int64_t reg1,int64_t reg2);
void assemble_comp_reg_reg_shift(uint32_t opcode,uint8_t flags,int64_t reg1,int64_t reg2,uint8_t shift_type,uint8_t shift_val);
void assemble_comp_reg_reg_shift_reg(uint32_t opcode,uint8_t flags,int64_t reg1,int64_t reg2,uint8_t shift_type,uint8_t shift_reg);


void assemble_data_proc_reg_imm(uint32_t opcode,uint8_t flags,uint8_t update_flags,int64_t reg,int64_t imm);
void assemble_data_proc_reg_reg(uint32_t opcode,uint8_t flags,uint8_t update_flags,int64_t reg1,int64_t reg2);
void assemble_data_proc_reg_reg_shift(uint32_t opcode,uint8_t flags,uint8_t update_flags,int64_t reg1,int64_t reg2,uint8_t shift_type,uint8_t shift_val);
void assemble_data_proc_reg_reg_shift_reg(uint32_t opcode,uint8_t flags,uint8_t update_flags,int64_t reg1,int64_t reg2,uint8_t shift_type,uint8_t shift_reg);


void assemble_data_proc_reg_reg_imm(uint32_t opcode,uint8_t flags,uint8_t update_flags,int64_t reg1, int64_t reg2,int64_t imm);
void assemble_data_proc_reg_reg_reg(uint32_t opcode,uint8_t flags,uint8_t update_flags,int64_t reg1,int64_t reg2,int64_t reg3);
void assemble_data_proc_reg_reg_reg_shift(uint32_t opcode,uint8_t flags,uint8_t update_flags,int64_t reg1,int64_t reg2,int64_t reg3,uint8_t shift_type,uint8_t shift_val);
void assemble_data_proc_reg_reg_reg_shift_reg(uint32_t opcode,uint8_t flags,uint8_t update_flags,int64_t reg1,int64_t reg2,int64_t reg3,uint8_t shift_type,uint8_t shift_reg);


int find_section(const char *name);
// returns a pointer to the label with the name, or NULL if not found
struct label* find_label(const char *name);

extern int current_section;
extern struct section** sections;

#endif