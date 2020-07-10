%{
	#include <stdint.h>
	#include "parser.tab.h"
	#include "definitions.h"
	extern int yylex();
	extern void yyerror(const char*);
	long char_count = 0;
	long line_count = 1;
%}
%option noyywrap
%%
";".*							{char_count += strlen(yytext); return COMMENT;}
0b[01]+							{char_count += strlen(yytext); yylval.integer = string_to_immediate(yytext+2,2); return INTEGER;}
"-"[0-9]+						{char_count += strlen(yytext); yylval.integer = string_to_immediate(yytext,10); return INTEGER;}
"-"0x[0-9a-fA-F]+				{char_count += strlen(yytext); yylval.integer = string_to_immediate(yytext,16); return INTEGER;}
[0-9]+							{char_count += strlen(yytext); yylval.integer = string_to_immediate(yytext,10); return INTEGER;}
0x[0-9a-fA-F]+					{char_count += strlen(yytext); yylval.integer = string_to_immediate(yytext,16); return INTEGER;}
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
[a-zA-Z_][a-zA-Z_\.]+":"		{char_count += strlen(yytext); if (strlen(yytext) > 48) {yyerror("string too long");}; strncpy(yylval.string,yytext,strlen(yytext)-1); return LABEL;} /* strlen(yytext)-1, because we don't need the ':' at the end */
[" ""	"]*\n[" ""	"]*			{int cnt = 0; while (1) {if (yytext[cnt] == '\0') {char_count = 0; break;};if (yytext[cnt] == '\n') {char_count = strlen(yytext+cnt+1); break;}; cnt++;}; line_count++; return '\n';} /* swallow whitespaces at the start of a line */
[" ""	"]*\r\n[" ""	"]*		{int cnt = 0; while (1) {if (yytext[cnt] == '\0') {char_count = 0; break;};if (yytext[cnt] == '\n') {char_count = strlen(yytext+cnt+1); break;}; cnt++;}; line_count++; return '\n';} /* swallow whitespaces at the start of a line */
[" ""	"]+						{char_count += strlen(yytext); return WHITESPACE;};
<<EOF>>							{ static int once = 0; return once++ ? 0 : '\n'; } /* to artificially add an extra newline to the end */
.								{char_count++; return *yytext;}
%%
#include "main.c"