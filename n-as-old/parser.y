%{
	#include <stdint.h>
	int yylex(void);
	void yyerror(char const *);
%}



%union {
	uint32_t integer; // a constant in the assembly language
	char* string;
	char* instruction; // opcode of a conditional
	
}
%token <string> STRING
%token <integer> INTEGER
%token <string> COMMENT
%token <instruction> INSTRUCTION

%%

input:
  %empty
| input line
;


line:
  '\n'
| section '\n'
| label statement '\n'
| label '\n'
| section COMMENT '\n'
| label statement COMMENT '\n'
| label COMMENT '\n'
;

label:
STRING':'
;

section:
'.'STRING
;


statement:
".word" INTEGER
| ".short" INTEGER
| ".byte" INTEGER
;







