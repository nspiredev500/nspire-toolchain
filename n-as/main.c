#include "definitions.h"
int main(int argc, char* argv[])
{
	assembler_flags = ASSEMBLER_SWI_ALLOWED | ASSEMBLER_PSR_ALLOWED | ASSEMBLER_COPROCESSOR_ALLOWED;
	
	//extern uint64_t allocations;
	//printf("allocations before: %llu\n",allocations);
	if (argc != 2 && argc != 3)
	{
		return -1;
	}
	int parse_ret = 0;
	if (argc == 3)
	{
		yyin = stdin;
		parse_ret = yyparse();
	}
	else
	{
		YY_BUFFER_STATE s = yy_scan_string(argv[1]);
		parse_ret = yyparse();
		yy_delete_buffer(s);
	}
	yylex_destroy(); // on calc, only do this when the library handle is freed, as the lexer is unusable unless restarted once this runs
	if (current_section != -1 && parse_ret == 0)
	{
		int size = arrange_sections();
		void* binary = malloc(size);
		if (binary == NULL)
		{
			printf("Out of Memory!\n");
			return -1;
		}
		memset(binary,0,size);
		if (apply_fixups())
		{
			assemble_binary(binary,size);
			FILE* f = fopen("sectiondump","wb");
			if (f != NULL)
			{
				fwrite(binary,1,size,f);
				fclose(f);
			}
		}
	}
	else
	{
		return -1; // return with error for now, so the makefile stops processing
	}
	free_data();
	//printf("allocations after: %llu\n",allocations);
	/*
	extern uint64_t allocations;
	printf("allocations: %llu\n",allocations);
	*/
	return 0;
}
void yyerror(const char* error)
{
	printf("%s: at %d:%d\n",error,line_count,char_count);
	//exit(-1);
}