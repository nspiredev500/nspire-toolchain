%{
	#include <stdint.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include "parser.tab.h"
	extern int yylex();
	long char_count = 0;
%}
%option noyywrap
%%
";".*					{char_count += strlen(yytext); return COMMENT;}
[0-9]+					{char_count += strlen(yytext); yylval.integer = atoi(yytext); return INTEGER;}
0x[0-9a-fA-F]+			{char_count += strlen(yytext); yylval.integer = strtol(yytext,NULL,16); return INTEGER;}
0b[01]+					{char_count += strlen(yytext); yylval.integer = strtol(yytext,NULL,2); return INTEGER;}
("add"|"sub"|"mov"|"b"|"bl"|"blx"|"ldr"|"str")(("eq")|("ne")|("cs")|("cc")|("hs")|("lo")|("mi")|("pl")|("vs")|("vc")|("hi")|("ls")|("ge")|("lt")|("gt")|("le"))*	{char_count += strlen(yytext); yylval.instruction = strdup(yytext); return INSTRUCTION;}
[a-zA-Z_][a-zA-Z_\.]+			{char_count += strlen(yytext); yylval.string = strdup(yytext); return STRING;}
\.						{char_count++; return '.';}
\n						{char_count++; return '\n';}
":"						{char_count++; return ':';}
<<EOF>>					{ static int once = 0; return once++ ? 0 : '\n'; } /* to artificially add an extra newline to the end */
%%
#include "main.c"