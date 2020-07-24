#include "definitions.h"


//#define ON_CALC 1

#ifdef ON_CALC
	#include <libndls.h>
	#include <nspireio2.h>
#endif
int main(int argc, char* argv[])
{
	assembler_flags = ASSEMBLER_SWI_ALLOWED | ASSEMBLER_PSR_ALLOWED | ASSEMBLER_COPROCESSOR_ALLOWED;
	
	//extern uint64_t allocations;
	//printf("allocations before: %llu\n",allocations);
	int parse_ret = 0;
	#ifndef ON_CALC
		if (argc != 2 && argc != 3)
		{
			return -1;
		}
		if (argc == 3)
		{
			yyin = stdin;
			yyparse();
			parse_ret = assembler_error;
		}
		else
		{
			YY_BUFFER_STATE s = yy_scan_string(argv[1]);
			yyparse();
			parse_ret = assembler_error;
			yy_delete_buffer(s);
		}
	#else
		uart_printf("on-calc!\n");
		YY_BUFFER_STATE s = yy_scan_string(".text\n moveq r2, r1\nmov r1, #4\n add r0, r2, r1");
		yyparse();
		parse_ret = assembler_error;
		yy_delete_buffer(s);
	#endif
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
			#ifdef ON_CALC
				bkpt();
				void (*bin)(void) = (void(*)(void)) binary;
				bin();
				bkpt();
			#else
				FILE* f = fopen("sectiondump","wb");
				if (f != NULL)
				{
					fwrite(binary,1,size,f);
					fclose(f);
				}
			#endif
		}
		else
		{
			return -1; // return with error for now, so the makefile stops processing
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