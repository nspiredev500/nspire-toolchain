%{
	#include <stdint.h>
	#include "logic.c"
	#include <math.h>
	int yylex(void);
	void yyerror(char const *);
%}



%union {
	uint32_t integer; /* a constant in the assembly language */
	char string[50]; // a temporary buffer, so we don't need to free anything
	uint8_t condition_t;
	uint8_t update_flags;
	uint32_t opcode;
	uint8_t shift_t;
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
%nterm <opcode> testing_inst
%nterm <opcode> data_proc
%nterm <shift_t> shift;

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
"eq"		{$$ = 0;}
| "ne"		{$$ = 1;}
| "cs"		{$$ = 2;}
| "hs"		{$$ = 2;}
| "cc"		{$$ = 3;}
| "lo"		{$$ = 3;}
| "mi"		{$$ = 4;}
| "pl"		{$$ = 5;}
| "vs"		{$$ = 6;}
| "vc"		{$$ = 7;}
| "hi"		{$$ = 8;}
| "ls"		{$$ = 9;}
| "ge"		{$$ = 10;}
| "lt"		{$$ = 11;}
| "gt"		{$$ = 12;}
| "le"		{$$ = 13;}
| "al"		{$$ = 14;}
| %empty	{$$ = 14;}
;

update_flags:
"s"			{$$ = 1;}
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


statement:
DOTLONG WHITESPACE INTEGER			{if ($3 >= pow(2,32)) {yyerror("constant too big");}; section_write(current_section,&$3,4,-1);printf("word assembled\n");}
| DOTWORD WHITESPACE INTEGER		{if ($3 >= pow(2,32)) {yyerror("constant too big");}; section_write(current_section,&$3,4,-1);printf("word assembled\n");}
| DOTSHORT WHITESPACE INTEGER		{if ($3 >= pow(2,16)) {yyerror("constant too big");}; section_write(current_section,&$3,2,-1);printf("short assembled\n");}
| DOTBYTE WHITESPACE INTEGER		{if ($3 >= pow(2,8)) {yyerror("constant too big");}; section_write(current_section,&$3,1,-1);printf("byte assembled\n");}
| DOTARM				{arm = true;}
| DOTTHUMB				{arm = false;}

| testing_inst conditional WHITESPACE REGISTER delimiter '#' INTEGER		{assemble_comp_reg_imm($1,$2,$4,$7);}
| testing_inst conditional WHITESPACE REGISTER delimiter REGISTER			{assemble_comp_reg_reg($1,$2,$4,$6);}
| testing_inst conditional WHITESPACE REGISTER delimiter REGISTER delimiter shift WHITESPACE '#' INTEGER			{assemble_comp_reg_reg_shift($1,$2,$4,$6,$8,$11);}
| testing_inst conditional WHITESPACE REGISTER delimiter REGISTER delimiter shift WHITESPACE REGISTER				{assemble_comp_reg_reg_shift_reg($1,$2,$4,$6,$8,$10);}



| mov conditional update_flags WHITESPACE REGISTER delimiter '#' INTEGER	{assemble_data_proc_reg_imm($1,$2,$3,$5,$8);}
| mov conditional update_flags WHITESPACE REGISTER delimiter REGISTER		{assemble_data_proc_reg_reg($1,$2,$3,$5,$7);}
| mov conditional update_flags WHITESPACE REGISTER delimiter REGISTER delimiter shift WHITESPACE '#' INTEGER		{assemble_data_proc_reg_reg_shift($1,$2,$3,$5,$7,$9,$12);}
| mov conditional update_flags WHITESPACE REGISTER delimiter REGISTER delimiter shift WHITESPACE REGISTER	{assemble_data_proc_reg_reg_shift_reg($1,$2,$3,$5,$7,$9,$11);}
| mov conditional update_flags WHITESPACE REGISTER delimiter REGISTER delimiter 'r''r''x'		{assemble_data_proc_reg_reg_shift_reg($1,$2,$3,$5,$7,0b111,0);}



| data_proc conditional update_flags WHITESPACE REGISTER delimiter REGISTER delimiter '#' INTEGER		{assemble_data_proc_reg_reg_imm($1,$2,$3,$5,$7,$10);}
| data_proc conditional update_flags WHITESPACE REGISTER delimiter REGISTER delimiter REGISTER		{assemble_data_proc_reg_reg_reg($1,$2,$3,$5,$7,$9);}
| data_proc conditional update_flags WHITESPACE REGISTER delimiter REGISTER delimiter REGISTER delimiter shift WHITESPACE '#' INTEGER		{assemble_data_proc_reg_reg_reg_shift($1,$2,$3,$5,$7,$9,$11,$14);}
| data_proc conditional update_flags WHITESPACE REGISTER delimiter REGISTER delimiter REGISTER delimiter shift WHITESPACE REGISTER		{assemble_data_proc_reg_reg_reg_shift_reg($1,$2,$3,$5,$7,$9,$11,$13);}
| data_proc conditional update_flags WHITESPACE REGISTER delimiter REGISTER delimiter REGISTER delimiter 'r''r''x'		{assemble_data_proc_reg_reg_reg_shift_reg($1,$2,$3,$5,$7,$9,0b111,0);}



;


/*
statement:
DOTLONG INTEGER			{if ($2 >= pow(2,32)) {yyerror("constant too big");}; section_write(current_section,&$2,4,-1);printf("word assembled\n");}
| DOTWORD INTEGER		{if ($2 >= pow(2,32)) {yyerror("constant too big");}; section_write(current_section,&$2,4,-1);printf("word assembled\n");}
| DOTSHORT INTEGER		{if ($2 >= pow(2,16)) {yyerror("constant too big");}; section_write(current_section,&$2,2,-1);printf("short assembled\n");}
| DOTBYTE INTEGER		{if ($2 >= pow(2,8)) {yyerror("constant too big");}; section_write(current_section,&$2,1,-1);printf("byte assembled\n");}
| DOTARM				{arm = true;}
| DOTTHUMB				{arm = false;}
| INSTRUCTION REGISTER delimiter REGISTER delimiter REGISTER				{instruction_register_register_register($1,$2,$4,$6);}
| INSTRUCTION REGISTER delimiter '#' INTEGER								{instruction_register_int($1,$2,$5);}
| INSTRUCTION REGISTER delimiter REGISTER									{instruction_register_register($1,$2,$4);}
| INSTRUCTION REGISTER delimiter '[' REGISTER ']'							{instruction_register_memory_register($1,$2,$5,0);}
| INSTRUCTION REGISTER delimiter '[' REGISTER delimiter '#' INTEGER ']'		{instruction_register_memory_register($1,$2,$5,$8);}
| INSTRUCTION REGISTER delimiter '[' LABEL ']'								{instruction_register_memory_label($1,$2,$5);}
;
*/








