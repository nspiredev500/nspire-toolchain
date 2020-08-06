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

extern int assembler_error;

struct section {
	char* name; // must be freed
	uint32_t offset; // offset from the start of the binary. one section is selected as the start of the binary, and all others are relative to it
	void* data; // the contents of the section
	uint32_t size; // size of the section at this time
	uint32_t nextindex; // if (data+nextindex) > size and something has to be put in the section, the section is made bigger
};

#define FIXUP_B 0
#define FIXUP_BLX 1
#define FIXUP_MEM_W_B 2
#define FIXUP_MEM_H 3
#define FIXUP_MEM_W_B_IMM 6
#define FIXUP_MEM_W_B_ADDR 7
#define FIXUP_LABEL_ADDR_REL 8

#define FIXUP_FIXED 9

#define FIXUP_THUMB_B_COND 10
#define FIXUP_THUMB_B 11
#define FIXUP_THUMB_MEM 12
#define FIXUP_THUMB_MEM_IMM 13
#define FIXUP_THUMB_MEM_ADDR 14



struct fixup {
	char* name; // must be freed
	uint32_t offset; // offset from the beginning of the section
	uint8_t fixup_type; // define show the fixup is applied
	int16_t section; // index into the section table
	uint32_t extra; // the immediate value of FIXUP_MEM_IMM is stored here, and the addend for FIXUP_LABEL_ADDR_REL
};


struct label {
	uint32_t offset; // offset from the beginning of the section
	char* name; // must be freed
	int16_t section; // index into the section table. -1 if only defined and not yet found
};


enum condition {
	EQ=0,NE,CSHS,CCLO,MI,PL,VS,VC,HI,LS,GE,LT,GT,LE,ALWAYS,NEVER // IMPORTANT: NEVER is not allowed in arm9, the behavior is UNPREDICTABLE 
};
enum condition get_condition(char* str,int);

bool add_section(struct section *sect);
bool add_fixup(struct fixup *fix);
bool add_label(struct label *l);

void label_defined(char* label);


int arrange_sections(); // returns the size of the full binary
bool apply_fixups();
void assemble_binary(void* binary,int max);




// returns a pointer to the label with the name, or NULL if not found
struct label* find_label(const char *name);

void section_read(void* dest,int sect, int size,int offset);

bool section_write(int sect,const void* data,uint32_t size,int offset);

// frees all labels, fixups and sections
void free_data();

enum addressing_mode {
	pre_indexed_addressing_mode, post_indexed_addressing_mode,offset_addressing_mode
};

#include "assembler.h"

extern uint16_t assembler_flags;

uint16_t register_range(int64_t r1, int64_t r2);

int64_t string_to_immediate(char* str,int base);

void next_pool_found();


void assemble_msr_imm(uint8_t flags,uint8_t spsr,uint8_t psr_fields, int64_t imm);
void assemble_msr_reg(uint8_t flags,uint8_t spsr,uint8_t psr_fields, int64_t reg);

void assemble_mrs(uint8_t flags, int64_t reg,uint8_t spsr);

void assemble_blx_reg(uint8_t flags, int64_t reg);
void assemble_blx_imm(int64_t imm);
void assemble_bx(uint8_t flags, int64_t reg);

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