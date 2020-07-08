%{
	#include <stdint.h>
	#include "parser.tab.h"
	extern int yylex();
	extern void yyerror(const char*);
	long char_count = 0;
	long line_count = 1;
%}
%option noyywrap
%%
";".*							{char_count += strlen(yytext); return COMMENT;}
"-"[0-9]+						{char_count += strlen(yytext); yylval.integer = - strtol(yytext+1,NULL,10); return INTEGER;}
"-"0x[0-9a-fA-F]+				{char_count += strlen(yytext); yylval.integer = - strtol(yytext+1,NULL,16); return INTEGER;}
"-"0b[01]+						{char_count += strlen(yytext); yylval.integer = - strtol(yytext+1,NULL,2); return INTEGER;}
[0-9]+							{char_count += strlen(yytext); yylval.integer = strtol(yytext,NULL,10); return INTEGER;}
0x[0-9a-fA-F]+					{char_count += strlen(yytext); yylval.integer = strtol(yytext,NULL,16); return INTEGER;}
0b[01]+							{char_count += strlen(yytext); yylval.integer = strtol(yytext,NULL,2); return INTEGER;}
("add"|"sub"|"mov"|"b"|"bl"|"blx"|"ldr"|"str")(("eq")|("ne")|("cs")|("cc")|("hs")|("lo")|("mi")|("pl")|("vs")|("vc")|("hi")|("ls")|("ge")|("lt")|("gt")|("le"))*	{char_count += strlen(yytext); if (strlen(yytext) > 48) {yyerror("string too long");}; strcpy(yylval.string,yytext); return INSTRUCTION;}
"cpsr"							{return CPSR;}
"spsr"							{return SPSR;}
"r"[0-9]{1,2}					{yylval.integer = atoi(yytext+1); if (yylval.integer > 15) {yyerror("invalid register");}; return REGISTER;}
".arm"							{char_count += strlen(yytext); return DOTARM;}
".thumb"						{char_count += strlen(yytext); return DOTTHUMB;}
".long"							{char_count += strlen(yytext); return DOTLONG;}
".word"							{char_count += strlen(yytext); return DOTWORD;}
".short"						{char_count += strlen(yytext); return DOTSHORT;}
".byte"							{char_count += strlen(yytext); return DOTBYTE;}
\.[a-zA-Z_\.]+					{char_count += strlen(yytext); if (strlen(yytext) > 48) {yyerror("string too long");}; strcpy(yylval.string,yytext); return SECTION;}
[a-zA-Z_][a-zA-Z_\.]+			{char_count += strlen(yytext); if (strlen(yytext) > 48) {yyerror("string too long");}; strcpy(yylval.string,yytext); return STRING;}
\.								{char_count++; return '.';}
\n								{char_count = 0; line_count++; return '\n';}
":"								{char_count++; return ':';}
[" ""	"]+						;
<<EOF>>							{ static int once = 0; return once++ ? 0 : '\n'; } /* to artificially add an extra newline to the end */
.								{char_count++; return *yytext;}
%%
#include "main.c"