%{
	#include <stdint.h>
	#include "logic.c"
	#include <math.h>
	int yylex(void);
	void yyerror(char const *);
%}



%union {
	int64_t integer; /* a constant in the assembly language */
	char string[50]; // a temporary buffer, so we don't need to free anything
}
%token <string> STRING
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




%%

input:
  %empty
| input line
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
| section COMMENT
| label COMMENT
| statement COMMENT
| label statement
;


endline:
'\n'
| '\r''\n'
;


label:
STRING':'		{label_encountered($1);}
;

section:
SECTION		{section_encountered($1);}
;

delimiter:
','
;
/* TODO: copy the strings into a list in flex, as each copy of any data type in bison will take 50 bytes on it's stack now  */
/* the list can then be freed after the parsing is complete */
statement:
DOTLONG INTEGER			{if ($2 >= pow(2,32)) {yyerror("constant too big");}; section_write(current_section,&$2,4,-1);printf("word assembled\n");}
| DOTWORD INTEGER		{if ($2 >= pow(2,32)) {yyerror("constant too big");}; section_write(current_section,&$2,4,-1);printf("word assembled\n");}
| DOTSHORT INTEGER		{if ($2 >= pow(2,16)) {yyerror("constant too big");}; section_write(current_section,&$2,2,-1);printf("short assembled\n");}
| DOTBYTE INTEGER		{if ($2 >= pow(2,8)) {yyerror("constant too big");}; section_write(current_section,&$2,1,-1);printf("byte assembled\n");}
| DOTARM				{arm = true;}
| DOTTHUMB				{arm = false;}
| INSTRUCTION REGISTER delimiter REGISTER delimiter REGISTER	{instruction_register_register_register($1,$2,$4,$6);}
| INSTRUCTION REGISTER delimiter '#' INTEGER					{instruction_register_int($1,$2,$5);}
| INSTRUCTION REGISTER delimiter REGISTER						{instruction_register_register($1,$2,$4);}
| INSTRUCTION REGISTER delimiter '[' REGISTER ']'				{instruction_register_memory_register($1,$2,$5);}
| INSTRUCTION REGISTER delimiter '[' STRING ']'					{instruction_register_memory_label($1,$2,$5);}
;









