%{
	#include <stdint.h>
	#include <stdbool.h>
	#include "logic.c"
	#include <math.h>
	int yylex(void);
	void yyerror(char const *);
%}



%union {
	int64_t integer; /* a constant in the assembly language */
	char string[50]; /* a temporary buffer, so we don't need to free anything */
	uint8_t condition_t;
	uint8_t update_flags;
	uint32_t opcode;
	uint8_t shift_t;
	uint8_t mem_load;
	uint8_t width_t;
	uint16_t reglist_t;
}
%token <string> LABEL
%token <integer> INTEGER
%token COMMENT
%token <string> INSTRUCTION
%token CPSR
%token SPSR
%token <integer> REGISTER
%token <string> SECTION
%token DOTWORD
%token DOTLONG
%token DOTSHORT
%token DOTBYTE
%token DOTARM
%token DOTTHUMB
%token WHITESPACE


%nterm <condition_t> conditional
%nterm <update_flags> update_flags
%nterm <opcode> mov
%nterm <opcode> swp_inst
%nterm <opcode> testing_inst
%nterm <opcode> data_proc
%nterm <opcode> coproc_inst
%nterm <shift_t> shift
%nterm <mem_load> mem_inst
%nterm <mem_load> mem_multiple
%nterm <width_t> byte
%nterm <width_t> user_mode
%nterm <update_flags> update_reg
%nterm <update_flags> opt_minus
%nterm <width_t> width_specifier
%nterm <update_flags> opt_l
%nterm <width_t> multiple_mode
%nterm <reglist_t> reglist
%nterm <update_flags> user_mode_regs
%nterm <integer> register
%nterm <integer> spr


%%

input:
  %empty
| input line
| input WHITESPACE line
;


line:
  endline
| content endline
;

content:
COMMENT
| section
| label
| statement
| label statement COMMENT
| label WHITESPACE statement COMMENT
| label statement WHITESPACE COMMENT
| label WHITESPACE statement WHITESPACE COMMENT
| section COMMENT
| label COMMENT
| statement COMMENT
| label statement
| section WHITESPACE COMMENT
| label WHITESPACE COMMENT
| statement WHITESPACE COMMENT
| label WHITESPACE statement
;


endline:
'\n'
| '\r''\n'
;


label:
LABEL		{label_encountered($1);}
;

section:
SECTION		{section_encountered($1);}
;

delimiter:
','
| WHITESPACE ','
| WHITESPACE ',' WHITESPACE
| ',' WHITESPACE
;



/* TODO: copy the strings into a list in flex, as each copy of any data type in bison will take 50 bytes on it's stack now  */
/* the list can then be freed after the parsing is complete */
/* .ltorg directive, to specify a safe place to place a literal pool for ldr pseudo-instructions to use */
/* or instead of literal pools use ldr rx, [pc]\n b -4\n .word value   */
/* conditianals: the movs instruction would get interpreted as mov vs  */
/* should be fixed with the start parameter of get_conditional */
/* now we only need to detect the s */


conditional:
'e''q'		{$$ = 0;}
| 'n''e'		{$$ = 1;}
| 'c''s'		{$$ = 2;}
| 'h''s'		{$$ = 2;}
| 'c''c'		{$$ = 3;}
| 'l''o'		{$$ = 3;}
| 'm''i'		{$$ = 4;}
| 'p''l'		{$$ = 5;}
| 'v''s'		{$$ = 6;}
| 'v''c'		{$$ = 7;}
| 'h''i'		{$$ = 8;}
| 'l''s'		{$$ = 9;}
| 'g''e'		{$$ = 10;}
| 'l''t'		{$$ = 11;}
| 'g''t'		{$$ = 12;}
| 'l''e'		{$$ = 13;}
| 'a''l'		{$$ = 14;}
| %empty	{$$ = 14;}
;

update_flags:
's'			{$$ = 1;}
| %empty	{$$ = 0;}
;



mov:
'm''o''v'	{$$ = 0b1101;}
| 'm''v''n'	{$$ = 0b1111;}
;







testing_inst:
'c''m''p'	{$$ = 0b1010;}
| 'c''m''n'	{$$ = 0b1011;}
| 't''s''t'	{$$ = 0b1000;}
| 't''e''q'	{$$ = 0b1001;}
;

data_proc:
'a''d''d'	{$$ = 0b0100;}		/* don't include mov and the testing instructions here, as they require different formatting */
| 's''u''b'	{$$ = 0b0010;}
| 'r''s''b'	{$$ = 0b0011;}
| 'a''d''c'	{$$ = 0b0101;}
| 's''b''c'	{$$ = 0b0110;}
| 'r''s''c'	{$$ = 0b0111;}
| 'a''n''d'	{$$ = 0b0000;}
| 'b''i''c'	{$$ = 0b1110;}
| 'e''o''r'	{$$ = 0b0001;}
| 'o''r''r'	{$$ = 0b1100;}
;

shift:
'a''s''r'	{$$ = 0b10;}
| 'l''s''l'	{$$ = 0b00;}
| 'l''s''r'	{$$ = 0b01;}
| 'r''o''r'	{$$ = 0b11;}
;

/*'r''r''x'	{$$ = 0b111;}  real value is 0b11, but 0b111 to distinguish it from ror   */


mem_inst:
'l''d''r'	{$$ = 1;}
| 's''t''r'	{$$ = 0;}
;

byte:
%empty	{$$ = 0;}
| 'b'	{$$ = 1;}
;


user_mode:
%empty	{$$ = 0;}
| 't'	{$$ = 1;}
;


opt_whitespace:
%empty
| WHITESPACE
;


update_reg:
%empty	{$$ = 0;}
| '!'	{$$ = 1;;}
;

opt_minus:
%empty	{$$ = 1;}
| '-'	{$$ = 0;}
;

rrx:
'r''r''x'
;


width_specifier:
'd'	{$$ = 1;}
| 'h'	{$$ = 2;}
| 's''h'	{$$ = 3;}
| 's''b'	{$$ = 4;}
;

opt_l:
%empty	{$$ = 0;}
| 'l'	{$$ = 1;}
;
mem_multiple:
'l''d''m'	{$$ = 1;}
| 's''t''m'	{$$ = 0;}
;

multiple_mode:
%empty	{$$ = 0;}	/* "ia" is the default */
| 'i''a'	{$$ = 0;}
| 'i''b'	{$$ = 1;}
| 'd''a'	{$$ = 2;}
| 'd''b'	{$$ = 3;}
;

register:
REGISTER {$$ = $1;}
| 's''p'	{$$ = 13;}
| 'l''r'	{$$ = 14;}
| 'p''c'	{$$ = 15;}
| 'i''p'	{$$ = 12;}
| 'f''p'	{$$ = 11;}
;

reglist:
register	{$$ = 1 << $1;}
| register opt_whitespace '-' opt_whitespace register	{$$ = register_range($1,$5);}
| reglist delimiter reglist		{$$ = $1 | $3;}
;

user_mode_regs:
%empty	{$$ = 0;}
| '^'	{$$ = 1;}
;


coproc_inst:
'm''r''c'	{$$ = 1;} /* defines bit 20 of the instruction */
| 'm''c''r'	{$$ = 0;}
;


swp_inst:
's''w''p'	{$$ = 0;}
| 's''w''p''b'	{$$ = 1;}
;




/* TODO: branch with labels */
/*       ldr and str with labels  */
/*       branch with labels */
/*       flags to enable coprocessor instructions, swi and cpsr instructions */
/*		 instructions: msr/mrs, bx/blx, mul* */


statement:
DOTLONG WHITESPACE INTEGER			{if ($3 >= pow(2,32)) {yyerror("constant too big");}; section_write(current_section,&$3,4,-1);printf("word assembled\n");}
| DOTWORD WHITESPACE INTEGER		{if ($3 >= pow(2,32)) {yyerror("constant too big");}; section_write(current_section,&$3,4,-1);printf("word assembled\n");}
| DOTSHORT WHITESPACE INTEGER		{if ($3 >= pow(2,16)) {yyerror("constant too big");}; section_write(current_section,&$3,2,-1);printf("short assembled\n");}
| DOTBYTE WHITESPACE INTEGER		{if ($3 >= pow(2,8)) {yyerror("constant too big");}; section_write(current_section,&$3,1,-1);printf("byte assembled\n");}
| DOTARM				{arm = true;}
| DOTTHUMB				{arm = false;}

| error {YYABORT;}





| swp_inst conditional WHITESPACE REGISTER delimiter REGISTER delimiter '[' opt_whitespace REGISTER opt_whitespace ']'	{assemble_swp($1,$2,$4,$6,$10);}


| coproc_inst conditional WHITESPACE 'p' INTEGER delimiter INTEGER delimiter register delimiter 'c' INTEGER delimiter 'c' INTEGER delimiter INTEGER	{assemble_coproc($1,$2,$5,$7,$9,$12,$15,$17);}


| 's''w''i' conditional WHITESPACE INTEGER		{assemble_swi($4,$6);}

| 'c''l''z' conditional WHITESPACE register delimiter register	{assemble_clz($4,$6,$8);}

| mem_multiple multiple_mode conditional WHITESPACE register opt_whitespace update_reg delimiter '{' opt_whitespace reglist opt_whitespace '}' opt_whitespace user_mode_regs	{assemble_mem_multiple($1,$2,$3,$5,$7,$11,$15);}



| 'b' opt_l conditional WHITESPACE '#' INTEGER	{assemble_branch($2,$3,$6);}


| mem_inst width_specifier conditional WHITESPACE register delimiter '[' opt_whitespace register opt_whitespace ']' opt_whitespace update_reg		{assemble_mem_half_signed_imm($1,$2,$3,$5,$9,0,offset_addressing_mode,$13);}
| mem_inst width_specifier conditional WHITESPACE register delimiter '[' opt_whitespace register delimiter '#' INTEGER ']' opt_whitespace update_reg		{assemble_mem_half_signed_imm($1,$2,$3,$5,$9,$12,pre_indexed_addressing_mode,$15);}
| mem_inst width_specifier conditional WHITESPACE register delimiter '[' opt_whitespace register ']' delimiter '#' INTEGER opt_whitespace update_reg		{assemble_mem_half_signed_imm($1,$2,$3,$5,$9,$13,post_indexed_addressing_mode,$15);}


| mem_inst width_specifier conditional WHITESPACE register delimiter '[' opt_whitespace register delimiter opt_minus opt_whitespace register ']' opt_whitespace update_reg		{assemble_mem_half_signed_reg_offset($1,$2,$3,$5,$9,$13,pre_indexed_addressing_mode,$16,$11);}
| mem_inst width_specifier conditional WHITESPACE register delimiter '[' opt_whitespace register ']' delimiter opt_minus opt_whitespace register opt_whitespace update_reg		{assemble_mem_half_signed_reg_offset($1,$2,$3,$5,$9,$14,pre_indexed_addressing_mode,$16,$12);}




| mem_inst byte user_mode conditional WHITESPACE register delimiter '[' opt_whitespace register opt_whitespace ']' opt_whitespace update_reg		{assemble_mem_word_ubyte_imm($1,$2,$3,$4,$6,$10,0,offset_addressing_mode,$14);}
| mem_inst byte user_mode conditional WHITESPACE register delimiter '[' opt_whitespace register delimiter '#' INTEGER ']' opt_whitespace update_reg		{assemble_mem_word_ubyte_imm($1,$2,$3,$4,$6,$10,$13,pre_indexed_addressing_mode,$16);}
| mem_inst byte user_mode conditional WHITESPACE register delimiter '[' opt_whitespace register opt_whitespace ']' delimiter '#' INTEGER opt_whitespace update_reg		{assemble_mem_word_ubyte_imm($1,$2,$3,$4,$6,$10,$15,post_indexed_addressing_mode,$17);}

| mem_inst byte user_mode conditional WHITESPACE register delimiter '[' opt_whitespace register delimiter opt_minus opt_whitespace register ']' opt_whitespace update_reg		{assemble_mem_word_ubyte_reg_offset($1,$2,$3,$4,$6,$10,$14,pre_indexed_addressing_mode,$17,$12);}
| mem_inst byte user_mode conditional WHITESPACE register delimiter '[' opt_whitespace register opt_whitespace ']' delimiter opt_minus opt_whitespace register opt_whitespace update_reg		{assemble_mem_word_ubyte_reg_offset($1,$2,$3,$4,$6,$10,$16,post_indexed_addressing_mode,$18,$14);}

| mem_inst byte user_mode conditional WHITESPACE register delimiter '[' opt_whitespace register delimiter opt_minus opt_whitespace register delimiter shift WHITESPACE '#' INTEGER ']' opt_whitespace update_reg		{assemble_mem_word_ubyte_reg_offset_scaled($1,$2,$3,$4,$6,$10,$14,$16,$19,pre_indexed_addressing_mode,$22,$12);}
| mem_inst byte user_mode conditional WHITESPACE register delimiter '[' opt_whitespace register opt_whitespace ']' delimiter opt_minus opt_whitespace register delimiter shift WHITESPACE '#' INTEGER opt_whitespace update_reg		{assemble_mem_word_ubyte_reg_offset_scaled($1,$2,$3,$4,$6,$10,$16,$18,$21,post_indexed_addressing_mode,$23,$14);}

| mem_inst byte user_mode conditional WHITESPACE register delimiter '[' opt_whitespace register opt_whitespace ']' delimiter opt_minus opt_whitespace register delimiter rrx opt_whitespace update_reg		{assemble_mem_word_ubyte_reg_offset_scaled($1,$2,$3,$4,$6,$10,$16,0b111,0,post_indexed_addressing_mode,$20,$14);}
| mem_inst byte user_mode conditional WHITESPACE register delimiter '[' opt_whitespace register delimiter opt_minus opt_whitespace register delimiter rrx ']' opt_whitespace update_reg		{assemble_mem_word_ubyte_reg_offset_scaled($1,$2,$3,$4,$6,$10,$14,0b111,0,pre_indexed_addressing_mode,$19,$12);}



| testing_inst conditional WHITESPACE register delimiter '#' INTEGER		{assemble_comp_reg_imm($1,$2,$4,$7);}
| testing_inst conditional WHITESPACE register delimiter register			{assemble_comp_reg_reg($1,$2,$4,$6);}
| testing_inst conditional WHITESPACE register delimiter register delimiter shift WHITESPACE '#' INTEGER			{assemble_comp_reg_reg_shift($1,$2,$4,$6,$8,$11);}
| testing_inst conditional WHITESPACE register delimiter register delimiter shift WHITESPACE register				{assemble_comp_reg_reg_shift_reg($1,$2,$4,$6,$8,$10);}



| mov update_flags conditional WHITESPACE register delimiter '#' INTEGER	{assemble_data_proc_reg_imm($1,$3,$2,$5,$8);}
| mov update_flags conditional WHITESPACE register delimiter register		{assemble_data_proc_reg_reg($1,$3,$2,$5,$7);}
| mov update_flags conditional WHITESPACE register delimiter register delimiter shift WHITESPACE '#' INTEGER		{assemble_data_proc_reg_reg_shift($1,$3,$2,$5,$7,$9,$12);}
| mov update_flags conditional WHITESPACE register delimiter register delimiter shift WHITESPACE register	{assemble_data_proc_reg_reg_shift_reg($1,$3,$2,$5,$7,$9,$11);}
| mov update_flags conditional WHITESPACE register delimiter register delimiter rrx		{assemble_data_proc_reg_reg_shift_reg($1,$3,$2,$5,$7,0b111,0);}



| data_proc update_flags conditional WHITESPACE register delimiter register delimiter '#' INTEGER		{assemble_data_proc_reg_reg_imm($1,$3,$2,$5,$7,$10);}
| data_proc update_flags conditional WHITESPACE register delimiter register delimiter register		{assemble_data_proc_reg_reg_reg($1,$3,$2,$5,$7,$9);}
| data_proc update_flags conditional WHITESPACE register delimiter register delimiter register delimiter shift WHITESPACE '#' INTEGER		{assemble_data_proc_reg_reg_reg_shift($1,$3,$2,$5,$7,$9,$11,$14);}
| data_proc update_flags conditional WHITESPACE register delimiter register delimiter register delimiter shift WHITESPACE register		{assemble_data_proc_reg_reg_reg_shift_reg($1,$3,$2,$5,$7,$9,$11,$13);}
| data_proc update_flags conditional WHITESPACE register delimiter register delimiter register delimiter rrx		{assemble_data_proc_reg_reg_reg_shift_reg($1,$3,$2,$5,$7,$9,0b111,0);}
;






