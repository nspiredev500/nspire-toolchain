%{
	#include <stdint.h>
	int yylex(void);
	void yyerror(char const *);
%}



%union {
	int integer; // a constant in the assembly language
	char* string;
	char* instruction; // opcode of a conditional
	
}
%token <string> STRING
%token <integer> INTEGER
%token COMMENT
%token <instruction> INSTRUCTION

%%

input:
  %empty
| input line
;


line:
  endline
| COMMENT endline
| section endline
| label statement endline
| label endline
| statement endline
| statement COMMENT endline
| section COMMENT endline
| label statement COMMENT endline
| label COMMENT endline
;

endline:
'\n'
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
| INSTRUCTION
;









