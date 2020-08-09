#include "definitions.h"
#include <errno.h>




//#define STANDALONE 1

#ifdef STANDALONE
	int main(int argc, char* argv[])
	{
		assembler_flags = ASSEMBLER_SWI_ALLOWED | ASSEMBLER_PSR_ALLOWED | ASSEMBLER_COPROCESSOR_ALLOWED;
		int parse_ret = 0;
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
				free(binary);
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
		return 0;
	}
	void yyerror(const char* error)
	{
		printf("%s: at %d:%d\n",error,line_count,char_count);
	}
#else
	/*
	int main()
	{
		uint32_t size = 0;
		void* block = NULL;
		assemble_string(".text\nmov r1,r2\n",0,&size,&block);
		if (block != NULL)
		{
			free(block);
		}
		assemble_string(".text\nmov r2,r3\n",0,&size,&block);
		printf("%s\n",asm_error_msg);
		FILE* f = fopen("sectiondump","wb");
		if (f != NULL)
		{
			fwrite(block,1,size,f);
			fclose(f);
		}
		else
		{
			printf("could not open file!\n");
			return -1;
		}
		if (block != NULL)
		{
			free(block);
		}
		return 0;
	}
	*/
	int assemble_string(const char* string, uint16_t flags, uint32_t* size_ret, void** mem)
	{
		memset(asm_error_msg,'\0',195);
		assembler_error = 0;
		current_section = -1;
		arm = true;
		
		int parse_ret = 0;
		assembler_flags = flags;
		YY_BUFFER_STATE s = yy_scan_string(string);
		yyparse();
		parse_ret = assembler_error;
		yy_delete_buffer(s);
		yylex_destroy();
		if (current_section != -1 && parse_ret == 0)
		{
			int size = arrange_sections();
			void* binary = malloc(size);
			if (binary == NULL)
			{
				*size_ret = 0;
				*mem = NULL;
				free_data();
				return ENOMEM;
			}
			memset(binary,0,size);
			if (apply_fixups())
			{
				assemble_binary(binary,size);
				*size_ret = size;
				*mem = binary;
				free_data();
				return 0;
			}
			else
			{
				free_data();
				*size_ret = 0;
				*mem = NULL;
				return -1;
			}
		}
		else
		{
			free_data();
			*size_ret = 0;
			*mem = NULL;
			return -1;
		}
	}
	void yyerror(const char* error)
	{
		snprintf(asm_error_msg,190,"%s",error);
	}
#endif