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

bool section_write(struct section* s,void* data,uint32_t size,int offset);

const char* error = NULL; // if a function sets this, and error occurred and is displayed

int linelength(const char* line)
{
	int count = 0;
	while (*line != '\0' && *line != '\n')
	{
		count++;
	}
	return count;
}


// one assemble function for every family of instructions
// line is a pointer into the assembly string, inst is a pointer to where the opcode should be placed
// the return value is the number of characters until after the next \n, or -1 if and \0 is encountered
// inst is a void pointer, because alignment can't be guaranteed by the assembly the user provides
int assemble_mov_arm(const char* line,void* inst);
int assemble_add_arm(const char* line,void* inst);
int assemble_sub_arm(const char* line,void* inst);
int assemble_ldr_arm(const char* line,void* inst);
int assemble_str_arm(const char* line,void* inst);
int assemble_tst_arm(const char* line,void* inst);
int assemble_cmp_arm(const char* line,void* inst);
int assemble_b_arm(const char* line,void* inst);
int assemble_word(const char* line,void* inst);
int assemble_short(const char* line,void* inst);
int assemble_byte(const char* line,void* inst);


// return value the same as above
int directive_section(const char* line,bool* thumb, uint32_t* current_section)
{
	int len = linelength(line);
	if (len == strlen(".arm"))
	{
		if (line[0] == '.' && line[1] == 'a' &&  line[2] == 'r' && line[3] == 'm')
		{
			/// TODO somehow take care about the alignment while not leaving holes in the binary
			/*
			if (((uint32_t) (sections[*current_section].nextindex) & 0b11) != 0)
			{
				sections[*current_section].nextindex &= ~0b11; // clear the lowest 2 bits
				sections[*current_section].nextindex += 0b100;
			}
			*/
			*thumb = false;
			if (line[len] == '\0')
			{
				return -1;
			}
			else
			{
				return len+1; // len points to the newline
			}
		}
	}
	if (len == strlen(".thumb"))
	{
		if (line[0] == '.' && line[1] == 't' &&  line[2] == 'h' && line[3] == 'u' && line[4] == 'm' && line[5] == 'b')
		{
			/// TODO somehow take care about the alignment while not leaving holes in the binary
			*thumb = true;
			if (line[len] == '\0')
			{
				return -1;
			}
			else
			{
				return len+1; // len points to the newline
			}
		}
	}
	if (len > 1)
	{
		if (line[0] == '.')
		{
			for (uint32_t i = 1;i<len;i++)
			{
				if (line[i] == ':')
				{
					if (line[len] == '\0')
					{
						return -1;
					}
					else
					{
						return len+1; // len points to the newline
					}
				}
			}
			struct section *s = malloc(struct section);
			if (s == NULL)
			{
				error = "Out of Memory!";
				return -1;
			}
			s->name = line;
			s->lenght = len;
			s->data = NULL;
			s->size = 0;
			s->nextindex = 0;
			add_section(s);
		}
	}
	error = "unknown directive";
	if (line[len] == '\0')
	{
		return -1;
	}
	else
	{
		return len+1; // len points to the newline
	}
}



struct section {
	const char* name; // pointer into the assembly string
	uint32_t length; // length of the name
	uint32_t offset; // offset from the start of the binary. one section is selected as the start of the binary, and all others are relative to it
	void* data; // the contents of the section
	uint32_t size; // size of the section at this time
	uint32_t nextindex; // if (data+nextindex) > size and something has to be put in the section, the section is made bigger
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


// if offset is -1, nextindex is used
bool section_write(struct section* s,void* data,uint32_t size,int offset)
{
	if (s->data == NULL)
	{
		s->data = malloc(50);
		if (s->data == NULL)
		{
			error = "Out of Memory!";
			return false;
		}
		s->size = 50;
	}
	if (offset == -1)
	{
		if (s->nextindex+size >= s->size)
		{
			uint32_t diff = ((s->nextindex+size)-s->size)+5;
			void* new_s_data = malloc(s->size+diff);
			if (new_s_data == NULL)
			{
				error = "Out of Memory!";
				return false;
			}
			memset(new_s_data,'\0',s->size+diff);
			memcpy(new_s_data,s->data,s->size);
			free(s->data);
			s->data = new_s_data;
		}
		memcpy(s->data,data,size);
		s->nextindex += size;
	}
	else
	{
		if (offset+size >= s->size)
		{
			uint32_t diff = ((offset+size)-s->size)+5;
			void* new_s_data = malloc(s->size+diff);
			if (new_s_data == NULL)
			{
				error = "Out of Memory!";
				return false;
			}
			memset(new_s_data,'\0',s->size+diff);
			memcpy(new_s_data,s->data,s->size);
			free(s->data);
			s->data = new_s_data;
		}
		memcpy(s->data,data,size);
	}
}

struct section **sections = NULL;
uint32_t sections_size = 0;
uint32_t next_section = 0;
struct fixup **fixups = NULL;
uint32_t fixups_size = 0;
uint32_t next_fixup = 0;
struct label **labels = NULL;
uint32_t labels_size = 0;
uint32_t next_label = 0;

bool add_section(struct section *sect)
{
	if (sections == NULL)
	{
		sections = malloc(sizeof(void*)*10);
		if (sections == NULL)
		{
			error = "Out of Memory!";
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
		struct section** new_sections = malloc(sections_size*2);
		if (new_sections == NULL)
		{
			error = "Out of Memory!";
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
	return true;
}

bool add_fixup(struct fixup *fix)
{
	if (fixups == NULL)
	{
		fixups = malloc(sizeof(void*)*10);
		if (fixups == NULL)
		{
			error = "Out of Memory!";
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
		struct fixup** new_fixups = malloc(fixups_size*2);
		if (new_fixups == NULL)
		{
			error = "Out of Memory!";
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
	return true;
}

bool add_label(struct label *l)
{
	if (labels == NULL)
	{
		labels = malloc(sizeof(void*)*10);
		if (labels == NULL)
		{
			error = "Out of Memory!";
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
		struct label** new_labels = malloc(labels_size*2);
		if (new_labels == NULL)
		{
			error = "Out of Memory!";
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
	return true;
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
		error = "Out of Memory!";
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
			error = "prepare_string: mod_index bigger than string length!\n";
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
	bool thumb = false;
	char* mod_str = prepare_string(string);
	if (mod_str == NULL)
	{
		return NULL;
	}
	printf("prepared string: %s\nend prepared string\n",mod_str);
	
	
	
	/// TODO default section is .text
	/// TODO if a section with the specified name exists, just select it in the section directive
	bool thumb = false;
	int index = 0;
	int ret = 0;
	while (ret != -1)
	{
		int len = linelength(mod_str+index);
		if (len > 1)
		{
			if (mod_str[index] == '.')
			{
				directive_section
			}
		}
		
	}
	
	
	
	
	
	
	
	if (sections != NULL)
	{
		free(sections);
	}
	if (fixups != NULL)
	{
		free(fixups);
	}
	if (labels != NULL)
	{
		free(labels);
	}
	free(mod_str);
	return NULL;
}









//
int main(void)
{
	
	assemble_string(".text\n.arm\n add r0, r0 #1");
	
	//asm("bkpt \n"
	//"test: .long 0x2800000");
	
	
	
	
	
	
	return 0;
}











