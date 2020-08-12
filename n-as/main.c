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
		bool thumb = false;
		void* block = NULL;
		uint32_t entry_offset = 0;
		assemble_string(".entry start\n.text\nstart: bkpt\nmov r0, #0\nmov r1, #1\nmov r3, #0\n push {r0}\npush {r1}\nloop: add r2, r1, r0\n push {r2}\nmov r0, r1\nmov r1, r2\nadd r3, r3, #1\n cmp r3, #10\n blt loop\n bx lr",0,&size,&block,&entry_offset,&thumb);
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
		void (*func)(void) = block;
		//func();
		if (block != NULL)
		{
			free(block);
		}
		//extern int max_allocations;
		//printf("max allocations: %d\n",max_allocations);
		return 0;
	}
	*/
	int assemble_string(const char* string, uint16_t flags, uint32_t* size_ret, void** mem,uint32_t* entry_offset, bool* thumb)
	{
		memset(asm_error_msg,'\0',195);
		memset(entry_label,'\0',49);
		assembler_error = 0;
		current_section = -1;
		arm = true;
		extern int eof_found;
		eof_found = 0;
		char_count = 0;
		line_count = 1;
		
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
			if (size == 0)
			{
				yyerror("empty program");
				free_data();
				*size_ret = 0;
				*mem = NULL;
				*entry_offset = 0;
				return -1;
			}
			void* binary = malloc(size);
			if (binary == NULL)
			{
				yyerror("Out of Memory!");
				*size_ret = 0;
				*mem = NULL;
				*entry_offset = 0;
				free_data();
				return ENOMEM;
			}
			memset(binary,0,size);
			if (apply_fixups())
			{
				assemble_binary(binary,size);
				struct label* entry = find_label(entry_label);
				if (strlen(entry_label) == 0)
				{
					yyerror("entry label not defined");
					free_data();
					*size_ret = 0;
					*mem = NULL;
					*entry_offset = 0;
					free(binary);
					return -1;
				}
				if (entry == NULL)
				{
					yyerror("entry label not found");
					free_data();
					*size_ret = 0;
					*mem = NULL;
					*entry_offset = 0;
					free(binary);
					return -1;
				}
				if (entry->section == -1)
				{
					yyerror("entry label not defined");
					free_data();
					*size_ret = 0;
					*mem = NULL;
					*entry_offset = 0;
					free(binary);
					return -1;
				}
				if (entry->thumb)
				{
					*thumb = true;
				}
				else
				{
					*thumb = false;
				}
				*size_ret = size;
				*mem = binary;
				*entry_offset = sections[entry->section]->offset + entry->offset;
				free_data();
				return 0;
			}
			else
			{
				free_data();
				*size_ret = 0;
				*mem = NULL;
				*entry_offset = 0;
				return -1;
			}
		}
		else
		{
			if (parse_ret == 0)
			{
				yyerror("no section defined");
			}
			free_data();
			*size_ret = 0;
			*mem = NULL;
			*entry_offset = 0;
			return -1;
		}
	}
	void yyerror(const char* error)
	{
		snprintf(asm_error_msg,190,"%s: at %ld:%ld",error,line_count,char_count);
	}
#endif