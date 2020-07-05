#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>
#include <limits.h>
#include <float.h>

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>

// one assemble function for every family of instructions
// line is a pointer into the assembly string, inst is a pointer to where the opcode should be placed
// the return value is the number of characters until after the next \n, or -1 if and \0 is encountered
int assemble_mov_arm(const char* line,uint32_t *inst);
int assemble_add_arm(const char* line,uint32_t *inst);
int assemble_sub_arm(const char* line,uint32_t *inst);
int assemble_ldr_arm(const char* line,uint32_t *inst);
int assemble_str_arm(const char* line,uint32_t *inst);
int assemble_tst_arm(const char* line,uint32_t *inst);
int assemble_cmp_arm(const char* line,uint32_t *inst);
int assemble_b_arm(const char* line,uint32_t *inst);


struct section {
	const char* name; // pointer into the assembly string
	uint32_t length; // length of the name
	uint32_t offset; // offset from the start of the binary. one section is selected as the start of the binary, and all others are relative to it
	void* data; // the contents of the section
	uint32_t size; // size of the section at this time
	void* nextindex; // if (data-nextindex) > size and something has to be put in the section, the section is made bigger
};


struct fixup {
	const char* name; // pointer into the assembly string
	uint32_t length; // length of the name
	uint32_t offset; // offset from the beginning of the section
	uint8_t maxbits; // maximum number of bits for this fixup. if more are needed, an error is generated
	uint8_t section; // index into the section table
};


struct label {
	uint32_t offset; // offset from the beginning of the section
	const char* name; // pointer into the assembly string
	uint32_t length; // length of the name
	uint8_t section; // index into the section table
};

struct section *sections = NULL;
uint32_t sections_size = 0;
uint32_t next_section = 0;
struct fixup *fixups = NULL;
uint32_t fixups_size = 0;
uint32_t next_fixup = 0;
struct label *labels = NULL;
uint32_t labels_size = 0;
uint32_t next_label = 0;

bool add_section(struct section *sect)
{
	if (sections == NULL)
	{
		sections = malloc(sizeof(void*)*10);
		if (sections == NULL)
		{
			return false;
		}
		sections_size = 10;
		for (uint32_t i = 0;i<sections_size;i++)
		{
			sections[i] = NULL;
		}
	}
	if (next_section >= sections_size)
	{
		struct section* new_sections = malloc(sections_size*2);
		if (new_sections == NULL)
		{
			return false;
		}
		for (uint32_t i = 0;i<sections_size*2;i++)
		{
			new_sections[i] = NULL;
		}
		for (uint32_t i = 0;i<sections_size;i++)
		{
			new_sections[i] = sections[i];
		}
		free(sections);
		sections = new_sections;
		sections_size = sections_size*2;
	}
	sections[next_section] = sect;
}

bool add_fixup(struct fixup *fix)
{
	if (fixups == NULL)
	{
		fixups = malloc(sizeof(void*)*10);
		if (fixups == NULL)
		{
			return false;
		}
		fixups_size = 10;
		for (uint32_t i = 0;i<fixups_size;i++)
		{
			fixups[i] = NULL;
		}
	}
	if (next_fixup >= fixups_size)
	{
		struct fixup* new_fixups = malloc(fixups_size*2);
		if (new_fixups == NULL)
		{
			return false;
		}
		for (uint32_t i = 0;i<fixups_size*2;i++)
		{
			new_fixups[i] = NULL;
		}
		for (uint32_t i = 0;i<fixups_size;i++)
		{
			new_fixups[i] = fixups[i];
		}
		free(fixups);
		fixups = new_fixups;
		fixups_size = fixups_size*2;
	}
	fixups[next_fixup] = fix;
}

bool add_label(struct label *l)
{
	if (labels == NULL)
	{
		labels = malloc(sizeof(void*)*10);
		if (labels == NULL)
		{
			return false;
		}
		labels_size = 10;
		for (uint32_t i = 0;i<labels_size;i++)
		{
			labels[i] = NULL;
		}
	}
	if (next_label >= labels_size)
	{
		struct label* new_labels = malloc(labels_size*2);
		if (new_labels == NULL)
		{
			return false;
		}
		for (uint32_t i = 0;i<labels_size*2;i++)
		{
			new_labels[i] = NULL;
		}
		for (uint32_t i = 0;i<labels_size;i++)
		{
			new_labels[i] = labels[i];
		}
		free(labels);
		labels = new_labels;
		labels_size = labels_size*2;
	}
	labels[next_label] = l;
}


// removes comments and empty lines from the string, and returns the modified string
// also removes leading whitespaces from lines
char* prepare_string(const char* string)
{
	if (string == NULL)
	{
		return NULL;
	}
	char* mod_str = malloc(strlen(string)+1); // count the null byte; our string will only be cut down, so it cannot be bigger than the input
	if (mod_str == NULL)
	{
		return NULL;
	}
	memset(mod_str,'\0',strlen(string)+1);
	uint32_t mod_index = 0;
	bool comment = false;
	bool empty = true;
	for (uint32_t i = 0;string[i] != '\0';i++)
	{
		if (mod_index >= strlen(string)+1)
		{
			printf("prepare_string: mod_index bigger than string length!\n");
			free(mod_str);
			return NULL;
		}
		char c = string[i];
		if (c == '\n')
		{
			if (! empty)
			{
				mod_str[mod_index] = c;
				mod_index++;;
			}
			empty = true;
			comment = false;
			continue;
		}
		if (c == ';')
		{
			comment = true;
		}
		if (comment)
		{
			continue; // don't copy comments
		}
		if (c != ' ' && c != '	')
		{
			empty = false;
		}
		if (empty)
		{
			continue;
		}
		mod_str[mod_index] = c;
		mod_index++;;
	}
	mod_index--;
	while (mod_str[mod_index] == '\n' && mod_index > 0)
	{
		mod_str[mod_index] = '\0'; // remove trailing newlines
		mod_index--;
	}
	/*
	printf("mod_index: %d\n",mod_index);
	for (uint32_t i = 0;mod_str[i] != '\0';i++)
	{
		if (mod_str[i] == '\n')
		{
			printf("newline found: %d!\n",i);
		}
	}
	*/
	return mod_str;
}

// return false if failed
bool do_fixup()
{
	
	
	
	return true;
}

void* assemble_string(const char* string)
{
	struct label *labels = NULL;
	bool thumb = false;
	char* mod_str = prepare_string(string);
	if (mod_str == NULL)
	{
		return NULL;
	}
	printf("prepared string: %s\nend prepared string\n",mod_str);
	
	
	
	
	
	
	
	
	
	free(mod_str);
	return NULL;
}









//
int main(void)
{
	
	assemble_string("  		mov r0, #1\n \n\n\n\n	\n");
	
	//asm("bkpt \n"
	//"test: .long 0x2800000");
	
	
	
	
	
	
	return 0;
}











