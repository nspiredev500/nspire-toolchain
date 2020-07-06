%{
	#include <stdint.h>
	#include <stdio.h>
	#include <stdlib.h>
	extern int yylex();
%}
%option noyywrap
%%
[\n]					printf("newline\n");
[0-9]+					printf("decimal number\n");
0x[0-9a-fA-F]+			printf("hex number\n");
0b[01]+					printf("binary number\n");
("add"|"sub"|"mov"|"b"|"bl"|"blx"|"ldr"|"str")(("eq")|("ne")|("cs")|("cc")|("hs")|("lo")|("mi")|("pl")|("vs")|("vc")|("hi")|("ls")|("ge")|("lt")|("gt")|("le"))*	printf("instruction\n");
[a-zA-Z_\.]+			printf("string\n");
%%
int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		return -1;
	}
	YY_BUFFER_STATE s = yy_scan_string(argv[1]);
	while (yylex());
	yy_delete_buffer(s);
	return 0;
}
void yyerror(const char* error)
{
	printf("%s",error);
	exit(-1);
}