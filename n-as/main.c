#include "definitions.h"
int main(int argc, char* argv[])
{
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
		yyparse();
	}
	else
	{
		YY_BUFFER_STATE s = yy_scan_string(argv[1]);
		parse_ret = yyparse();
		yy_delete_buffer(s);
	}
	yylex_destroy(); // on calc, only do this when the library handle is freed, as the lexer is unusable unless restarted once this runs
	if (current_section != -1 && parse_ret != 1)
	{
		FILE* f = fopen("sectiondump","wb");
		if (f != NULL)
		{
			fwrite(sections[current_section]->data,1,sections[current_section]->nextindex,f);
			fclose(f);
		}
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