%{
	#include <stdint.h>
	#include "parser.tab.h"
	#include "definitions.h"
	extern int yylex();
	extern void yyerror(const char*);
	long char_count = 0;
	long line_count = 1;
	int eof_found = 0;
	/* <<EOF>>							{ if (eof_found == 0) {eof_found++;return DOTPOOL;}; if (eof_found == 1) {eof_found++; return '\n';}; return 0;}  */
%}
%option noyywrap
%s DOTGLOBAL_FOUND
%%
";".*							{char_count += strlen(yytext); return COMMENT;}
<DOTGLOBAL_FOUND>[a-zA-Z_][a-zA-Z_\.]+			{BEGIN(0); if (strlen(yytext) > 48) {yyerror("string too long"); return 0;}; char_count += strlen(yytext); memcpy(yylval.string,yytext,yyleng);yylval.string[yyleng] = '\0'; return STRING;}
0b[01]+							{char_count += strlen(yytext); yylval.integer = string_to_immediate(yytext+2,2); if (yylval.integer == 0xffffffffff) return 0; return INTEGER;}
"-"[0-9]+						{char_count += strlen(yytext); yylval.integer = string_to_immediate(yytext,10); if (yylval.integer == 0xffffffffff) return 0; return INTEGER;}
"-"0x[0-9a-fA-F]+				{char_count += strlen(yytext); yylval.integer = string_to_immediate(yytext,16); if (yylval.integer == 0xffffffffff) return 0; return INTEGER;}
[0-9]+							{char_count += strlen(yytext); yylval.integer = string_to_immediate(yytext,10); if (yylval.integer == 0xffffffffff) return 0; return INTEGER;}
0x[0-9a-fA-F]+					{char_count += strlen(yytext); yylval.integer = string_to_immediate(yytext,16); if (yylval.integer == 0xffffffffff) return 0; return INTEGER;}
"cpsr"							{char_count += strlen(yytext); return CPSR;}
"spsr"							{char_count += strlen(yytext); return SPSR;}

"r"[0-9]{1,2}					{char_count += strlen(yytext); yylval.integer = atoi(yytext+1); if (yylval.integer > 15) {yyerror("invalid register"); return 0;}; return REGISTER;}
".global"[" ""	"]+				{char_count += strlen(yytext); BEGIN(DOTGLOBAL_FOUND); return DOTGLOBAL;}
".arm"							{char_count += strlen(yytext); return DOTARM;}
".thumb"						{char_count += strlen(yytext); return DOTTHUMB;}
".code 32"						{char_count += strlen(yytext); return DOTARM;}
".code 16"						{char_count += strlen(yytext); return DOTTHUMB;}
".long"							{char_count += strlen(yytext); return DOTLONG;}
".word"							{char_count += strlen(yytext); return DOTWORD;}
".short"						{char_count += strlen(yytext); return DOTSHORT;}
".byte"							{char_count += strlen(yytext); return DOTBYTE;}
".pool"							{char_count += strlen(yytext); return DOTPOOL;}
".ltorg"						{char_count += strlen(yytext); return DOTPOOL;}

".align"						{char_count += strlen(yytext); return DOTALIGN;}
".balign"						{char_count += strlen(yytext); return DOTALIGN;}
".asciz"						{char_count += strlen(yytext); return DOTASCIZ;}
".string"						{char_count += strlen(yytext); return DOTASCIZ;}
".ascii"						{char_count += strlen(yytext); return DOTASCII;}
".fill"							{char_count += strlen(yytext); return DOTFILL;}
".zero"							{char_count += strlen(yytext); return DOTZERO;}
".space"						{char_count += strlen(yytext); return DOTSPACE;}
".skip"							{char_count += strlen(yytext); return DOTSPACE;}


\.[a-zA-Z_\.]+					{char_count += strlen(yytext); if (strlen(yytext) > 48) {yyerror("string too long"); return 0;}; memcpy(yylval.string,yytext,yyleng);yylval.string[yyleng] = '\0'; return SECTION;}
[a-zA-Z_][a-zA-Z_\.]+":"		{char_count += strlen(yytext); if (strlen(yytext) > 48) {yyerror("string too long"); return 0;}; memcpy(yylval.string,yytext,yyleng-1);yylval.string[yyleng-1] = '\0'; return LABEL;} /* strlen(yytext)-1, because we don't need the ':' at the end */
[" ""	"]*\n[" ""	"]*			{BEGIN(0); int cnt = 0; while (1) {if (yytext[cnt] == '\0') {char_count = 0; break;};if (yytext[cnt] == '\n') {char_count = strlen(yytext+cnt+1); break;}; cnt++;}; line_count++; return '\n';} /* swallow whitespaces at the start of a line */
[" ""	"]*\r\n[" ""	"]*		{BEGIN(0); int cnt = 0; while (1) {if (yytext[cnt] == '\0') {char_count = 0; break;};if (yytext[cnt] == '\n') {char_count = strlen(yytext+cnt+1); break;}; cnt++;}; line_count++; return '\n';} /* swallow whitespaces at the start of a line */
[" ""	"]+						{char_count += strlen(yytext); return WHITESPACE;};
<<EOF>>							{ if (eof_found == 0) {eof_found++; return '\n';}; return 0;} 
.								{char_count++; return *yytext;}
%%
#include "main.c"