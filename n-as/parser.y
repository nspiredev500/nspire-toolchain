%{
	#include <stdint.h>
	#include <stdbool.h>
	#include "logic.c"
	#include <math.h>
	int yylex();
	void yyerror(const char *msg);
	void flex_search_string();
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
%token <string> STRING
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
%token DOTGLOBAL
%token DOTPOOL

%token DOTALIGN
%token DOTASCIZ
%token DOTASCII
%token DOTFILL
%token DOTZERO
%token DOTSPACE

%token DOTENTRY

%nterm <condition_t> conditional
%nterm <update_flags> update_flags
%nterm <opcode> mov
%nterm <opcode> swp_inst
%nterm <opcode> testing_inst
%nterm <opcode> data_proc
%nterm <opcode> coproc_inst
%nterm <opcode> mul_inst
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
%nterm <width_t> mul_width
%nterm <width_t> spr_fields
%nterm <width_t> opt_spr_fields
%nterm <string> string
%nterm <string> character
%nterm <opcode> thumb_data_proc
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
| thumb_data_proc	{$$ = $1;}
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

mul_inst:
'm''u''l'	{$$ = 0;}
| 'm''l''a'	{$$ = 1;}
| 's''m''u''l''l'	{$$ = 2;}
| 'u''m''u''l''l'	{$$ = 3;}
| 's''m''l''a''l'	{$$ = 4;}
| 's''m''l''a'		{$$ = 5;}
| 's''m''u''l''w'	{$$ = 6;}
| 's''m''u''l'		{$$ = 7;}
| 's''m''l''a''w'	{$$ = 8;}
;

mul_width:
%empty		{$$ = 0;}
| 'b'		{$$ = 1;}
| 't'		{$$ = 2;}
| 'b''b'	{$$ = 3;}
| 'b''t'	{$$ = 4;}
| 't''b'	{$$ = 5;}
| 't''t'	{$$ = 6;}
;

push:
'p''u''s''h'
;
pop:
'p''o''p'
;

spr:
SPSR	{$$ = 1;}
| CPSR	{$$ = 0;}
;



spr_fields:
'c'	{$$ = 0b1;}
| 'x'	{$$ = 0b10;}
| 's'	{$$ = 0b100;}
| 'f'	{$$ = 0b1000;}
| spr_fields spr_fields	{$$ = $1 | $2;}
;

opt_spr_fields:
%empty	{$$ = 0b1001;}
| '_' spr_fields	{$$ = $2;}

character:
'a' {$$[0] = 'a';$$[1] = '\0';} | 'b' {$$[0] = 'b';$$[1] = '\0';} | 'c' {$$[0] = 'c';$$[1] = '\0';} | 'd' {$$[0] = 'd';$$[1] = '\0';} | 'e' {$$[0] = 'e';$$[1] = '\0';} | 'f' {$$[0] = 'f';$$[1] = '\0';} | 'g' {$$[0] = 'g';$$[1] = '\0';} | 'h' {$$[0] = 'h';$$[1] = '\0';} | 'i' {$$[0] = 'i';$$[1] = '\0';} | 'j' {$$[0] = 'j';$$[1] = '\0';} | 'k' {$$[0] = 'k';$$[1] = '\0';} | 'l' {$$[0] = 'l';$$[1] = '\0';} | 'm' {$$[0] = 'm';$$[1] = '\0';} | 'n' {$$[0] = 'n';$$[1] = '\0';} | 'o' {$$[0] = 'o';$$[1] = '\0';} | 'p' {$$[0] = 'p';$$[1] = '\0';} | 'q' {$$[0] = 'q';$$[1] = '\0';} | 'r' {$$[0] = 'r';$$[1] = '\0';} | 's' {$$[0] = 's';$$[1] = '\0';} | 't' {$$[0] = 't';$$[1] = '\0';} | 'u' {$$[0] = 'u';$$[1] = '\0';} | 'v' {$$[0] = 'v';$$[1] = '\0';} | 'w' {$$[0] = 'w';$$[1] = '\0';} | 'x' {$$[0] = 'x';$$[1] = '\0';} | 'y' {$$[0] = 'y';$$[1] = '\0';} | 'z' {$$[0] = 'z';$$[1] = '\0';} |
'A' {$$[0] = 'A';$$[1] = '\0';} | 'B' {$$[0] = 'B';$$[1] = '\0';} | 'C' {$$[0] = 'C';$$[1] = '\0';} | 'D' {$$[0] = 'D';$$[1] = '\0';} | 'E' {$$[0] = 'E';$$[1] = '\0';} | 'F' {$$[0] = 'F';$$[1] = '\0';} | 'G' {$$[0] = 'G';$$[1] = '\0';} | 'H' {$$[0] = 'H';$$[1] = '\0';} | 'I' {$$[0] = 'I';$$[1] = '\0';} | 'J' {$$[0] = 'J';$$[1] = '\0';} | 'K' {$$[0] = 'K';$$[1] = '\0';} | 'L' {$$[0] = 'L';$$[1] = '\0';} | 'M' {$$[0] = 'M';$$[1] = '\0';} | 'N' {$$[0] = 'N';$$[1] = '\0';} | 'O' {$$[0] = 'O';$$[1] = '\0';} | 'P' {$$[0] = 'P';$$[1] = '\0';} | 'Q' {$$[0] = 'Q';$$[1] = '\0';} | 'R' {$$[0] = 'R';$$[1] = '\0';} | 'S' {$$[0] = 'S';$$[1] = '\0';} | 'T' {$$[0] = 'T';$$[1] = '\0';} | 'U' {$$[0] = 'U';$$[1] = '\0';} | 'V' {$$[0] = 'V';$$[1] = '\0';} | 'W' {$$[0] = 'W';$$[1] = '\0';} | 'X' {$$[0] = 'X';$$[1] = '\0';} | 'Y' {$$[0] = 'Y';$$[1] = '\0';} | 'Z' {$$[0] = 'Z';$$[1] = '\0';} |
'.' {$$[0] = '.';$$[1] = '\0';} | '_' {$$[0] = '_';$$[1] = '\0';}
;

string:
character character {$$[0] = $1[0];$$[1] = $2[0];$$[2] = '\0';}
| string character { if (strlen($1) < 46) { memcpy($$,$1,strlen($1)); $$[strlen($1)] = $2[0]; $$[strlen($1)+1] = '\0';} else {yyerror("string is too long");};}
;


/* TODO: branch with labels */
/*       assembler directives  */
/* 		 implementing .zero and .align */
/*			not sure if bl works correctly, especially the jump backwards. Test in a sample program */

thumb_data_proc:
'a''s''r'	{$$ = 0b10000;}
| 'l''s''l'	{$$ = 0b10001;}
| 'l''s''r'	{$$ = 0b10010;}
| 'n''e''g'	{$$ = 0b10011;}
| 'r''o''r'	{$$ = 0b10100;}
;



statement:
DOTLONG WHITESPACE INTEGER			{if ($3 >= pow(2,32)) {yyerror("constant too big");}; section_write(current_section,&$3,4,-1);}
| DOTWORD WHITESPACE INTEGER		{if ($3 >= pow(2,32)) {yyerror("constant too big");}; section_write(current_section,&$3,4,-1);}
| DOTSHORT WHITESPACE INTEGER		{if ($3 >= pow(2,16)) {yyerror("constant too big");}; section_write(current_section,&$3,2,-1);}
| DOTBYTE WHITESPACE INTEGER		{if ($3 >= pow(2,8)) {yyerror("constant too big");}; section_write(current_section,&$3,1,-1);}
| DOTARM				{arm = true;}
| DOTTHUMB				{arm = false;}
| DOTGLOBAL STRING	{label_defined($2,true);} /* DOTGLOBAL already eats up the whitespace */
| DOTPOOL		{next_pool_found();}

| DOTALIGN WHITESPACE INTEGER	{assemble_align($3);}
| DOTASCIZ WHITESPACE string	{assemble_ascii_zero($3);}
| DOTASCII WHITESPACE string	{assemble_ascii($3);}
| DOTFILL WHITESPACE INTEGER delimiter INTEGER delimiter INTEGER	{assemble_fill($3,$5,$7);}
| DOTZERO WHITESPACE INTEGER	{assemble_zero($3);}
| DOTSPACE WHITESPACE INTEGER delimiter INTEGER	{assemble_space($3,$5);}
| DOTSPACE WHITESPACE INTEGER	{assemble_space($3,0);}

| DOTENTRY WHITESPACE string	{assemble_entry($3);}


| error {assembler_error = -1;YYABORT;}

| 'u''d''f'	{assemble_undef();}
| 'u''n''d''e''f'	{assemble_undef();}
| 'b''k''p''t'	{assemble_bkpt();}

| mem_inst byte user_mode conditional WHITESPACE register delimiter '=' INTEGER		{assemble_mem_word_ubyte_imm_big($1,$2,$3,$4,$6,$9,offset_addressing_mode,0);}

| mem_inst byte user_mode conditional WHITESPACE register delimiter '=' string	{assemble_mem_word_ubyte_label_address($1,$2,$3,$4,$6,$9,offset_addressing_mode,0);}


| mem_inst width_specifier conditional WHITESPACE register delimiter string	{assemble_mem_half_signed_label($1,$2,$3,$5,$7,offset_addressing_mode,0);}
| mem_inst byte user_mode conditional WHITESPACE register delimiter string	{assemble_mem_word_ubyte_label($1,$2,$3,$4,$6,$8,offset_addressing_mode,0);}






| 'm''s''r' conditional WHITESPACE spr opt_spr_fields delimiter '#' INTEGER	{assemble_msr_imm($4,$6,$7,$10);}
| 'm''s''r' conditional WHITESPACE spr opt_spr_fields delimiter register	{assemble_msr_reg($4,$6,$7,$9);}

| 'm''r''s' conditional WHITESPACE register delimiter spr	{assemble_mrs($4,$6,$8);}

| 'b''l''x' conditional WHITESPACE register		{assemble_blx_reg($4,$6);}
| 'b''l''x' WHITESPACE register		{assemble_blx_reg(ALWAYS,$5);}
| 'b''l''x' WHITESPACE '#' INTEGER	{assemble_blx_imm($6);}

| 'b''x' conditional WHITESPACE register	{assemble_bx($3,$5);}


| 'b' opt_l conditional WHITESPACE string	{assemble_branch_label($2,$3,$5);}
| 'b''l''x' WHITESPACE string	{assemble_blx_label($5);}


| mul_inst mul_width update_flags conditional WHITESPACE register delimiter register						{assemble_mul($1,$2,$3,$4,$6,$8,-1,-1);}
| mul_inst mul_width update_flags conditional WHITESPACE register delimiter register delimiter register		{assemble_mul($1,$2,$3,$4,$6,$8,$10,-1);}
| mul_inst mul_width update_flags conditional WHITESPACE register delimiter register delimiter register delimiter register	{assemble_mul($1,$2,$3,$4,$6,$8,$10,$12);}


| swp_inst conditional WHITESPACE register delimiter register delimiter '[' opt_whitespace register opt_whitespace ']'	{assemble_swp($1,$2,$4,$6,$10);}


| coproc_inst conditional WHITESPACE 'p' INTEGER delimiter INTEGER delimiter register delimiter 'c' INTEGER delimiter 'c' INTEGER delimiter INTEGER	{assemble_coproc($1,$2,$5,$7,$9,$12,$15,$17);}


| 's''w''i' conditional WHITESPACE INTEGER		{assemble_swi($4,$6);}

| 'c''l''z' conditional WHITESPACE register delimiter register	{assemble_clz($4,$6,$8);}


| push conditional WHITESPACE  '{' opt_whitespace reglist opt_whitespace '}' opt_whitespace user_mode_regs	{assemble_mem_multiple(0,3,$2,13,1,$6,$10);}
| pop conditional WHITESPACE  '{' opt_whitespace reglist opt_whitespace '}' opt_whitespace user_mode_regs	{assemble_mem_multiple(1,0,$2,13,1,$6,$10);}

| mem_multiple multiple_mode conditional WHITESPACE register opt_whitespace update_reg delimiter '{' opt_whitespace reglist opt_whitespace '}' opt_whitespace user_mode_regs	{assemble_mem_multiple($1,$2,$3,$5,$7,$11,$15);}



| 'b' opt_l conditional WHITESPACE '#' INTEGER	{assemble_branch($2,$3,$6);}


| mem_inst width_specifier conditional WHITESPACE register delimiter '[' opt_whitespace register opt_whitespace ']' opt_whitespace update_reg		{assemble_mem_half_signed_imm($1,$2,$3,$5,$9,0,offset_addressing_mode,$13);}
| mem_inst width_specifier conditional WHITESPACE register delimiter '[' opt_whitespace register delimiter '#' INTEGER opt_whitespace ']' opt_whitespace update_reg		{assemble_mem_half_signed_imm($1,$2,$3,$5,$9,$12,pre_indexed_addressing_mode,$16);}
| mem_inst width_specifier conditional WHITESPACE register delimiter '[' opt_whitespace register opt_whitespace ']' delimiter '#' INTEGER opt_whitespace update_reg		{assemble_mem_half_signed_imm($1,$2,$3,$5,$9,$14,post_indexed_addressing_mode,$16);}


| mem_inst width_specifier conditional WHITESPACE register delimiter '[' opt_whitespace register delimiter opt_minus opt_whitespace register opt_whitespace ']' opt_whitespace update_reg		{assemble_mem_half_signed_reg_offset($1,$2,$3,$5,$9,$13,pre_indexed_addressing_mode,$17,$11);}
| mem_inst width_specifier conditional WHITESPACE register delimiter '[' opt_whitespace register opt_whitespace ']' delimiter opt_minus opt_whitespace register opt_whitespace update_reg		{assemble_mem_half_signed_reg_offset($1,$2,$3,$5,$9,$15,pre_indexed_addressing_mode,$17,$13);}




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


| data_proc update_flags conditional WHITESPACE register delimiter '#' INTEGER	{assemble_data_proc_reg_imm($1,$3,$2,$5,$8);}
| data_proc update_flags conditional WHITESPACE register delimiter register		{assemble_data_proc_reg_reg($1,$3,$2,$5,$7);}


| data_proc update_flags conditional WHITESPACE register delimiter register delimiter '#' INTEGER		{assemble_data_proc_reg_reg_imm($1,$3,$2,$5,$7,$10);}
| data_proc update_flags conditional WHITESPACE register delimiter register delimiter register		{assemble_data_proc_reg_reg_reg($1,$3,$2,$5,$7,$9);}
| data_proc update_flags conditional WHITESPACE register delimiter register delimiter register delimiter shift WHITESPACE '#' INTEGER		{assemble_data_proc_reg_reg_reg_shift($1,$3,$2,$5,$7,$9,$11,$14);}
| data_proc update_flags conditional WHITESPACE register delimiter register delimiter register delimiter shift WHITESPACE register		{assemble_data_proc_reg_reg_reg_shift_reg($1,$3,$2,$5,$7,$9,$11,$13);}
| data_proc update_flags conditional WHITESPACE register delimiter register delimiter register delimiter rrx		{assemble_data_proc_reg_reg_reg_shift_reg($1,$3,$2,$5,$7,$9,0b111,0);}
;



