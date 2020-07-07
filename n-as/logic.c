#include "definitions.h"



struct section **sections = NULL;
uint32_t sections_size = 0;
uint32_t next_section = 0;
struct fixup **fixups = NULL;
uint32_t fixups_size = 0;
uint32_t next_fixup = 0;
struct label **labels = NULL;
uint32_t labels_size = 0;
uint32_t next_label = 0;


int current_section = -1; // define a first section to use

bool arm = true; // false if thumb, gets reset to true if section is changed

// returns an index into the section table to the section with the name, or -1 if not found
int find_section(const char *name)
{
	for (uint32_t i = 0;i<sections_size;i++)
	{
		if (sections[i] != NULL)
		{
			if (strcmp(sections[i]->name,name) == 0 && strlen(sections[i]->name) == strlen(name))
			{
				return i;
			}
		}
	}
	return -1;
}


// returns a pointer to the label with the name, or NULL if not found
struct label* find_label(const char *name)
{
	for (uint32_t i = 0;i<labels_size;i++)
	{
		if (labels[i] != NULL)
		{
			if (strcmp(labels[i]->name,name) == 0 && strlen(labels[i]->name) == strlen(name))
			{
				return labels[i];
			}
		}
	}
	return NULL;
}



// if offset is -1, nextindex is used
bool section_write(int sect,void* data,uint32_t size,int offset)
{
	if (sect < 0 || sect >= sections_size)
	{
		yyerror("define a section first");
		return false;
	}
	struct section* s = sections[sect];
	if (s == NULL)
	{
		return false;
	}
	if (s->data == NULL)
	{
		s->data = malloc(50);
		if (s->data == NULL)
		{
			yyerror("Out of Memory!");
			return false;
		}
		s->size = 50;
	}
	if (offset == -1)
	{
		if (s->nextindex+size >= s->size)
		{
			uint32_t diff = ((s->nextindex+size)-s->size)+50;
			void* new_s_data = malloc(s->size+diff);
			if (new_s_data == NULL)
			{
				yyerror("Out of Memory!");
				return false;
			}
			memset(new_s_data,'\0',s->size+diff);
			memcpy(new_s_data,s->data,s->size);
			free(s->data);
			s->data = new_s_data;
			s->size += diff;
		}
		memcpy(s->data+s->nextindex,data,size);
		s->nextindex += size;
	}
	else
	{
		if (offset+size >= s->size)
		{
			uint32_t diff = ((offset+size)-s->size)+50;
			void* new_s_data = malloc(s->size+diff);
			if (new_s_data == NULL)
			{
				yyerror("Out of Memory!");
				return false;
			}
			memset(new_s_data,'\0',s->size+diff);
			memcpy(new_s_data,s->data,s->size);
			free(s->data);
			s->data = new_s_data;
			s->size += diff;
		}
		memcpy(s->data+offset,data,size);
	}
	return true;
}

bool add_section(struct section *sect)
{
	if (sections == NULL)
	{
		sections = malloc(sizeof(void*)*10);
		if (sections == NULL)
		{
			yyerror("Out of Memory!");
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
			yyerror("Out of Memory!");
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
	current_section = next_section;
	next_section++;
	return true;
}

bool add_fixup(struct fixup *fix)
{
	if (fixups == NULL)
	{
		fixups = malloc(sizeof(void*)*10);
		if (fixups == NULL)
		{
			yyerror("Out of Memory!");
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
			yyerror("Out of Memory!");
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
	next_fixup++;
	return true;
}

bool add_label(struct label *l)
{
	if (labels == NULL)
	{
		labels = malloc(sizeof(void*)*10);
		if (labels == NULL)
		{
			yyerror("Out of Memory!");
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
			yyerror("Out of Memory!");
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
	next_label++;
	return true;
}


void label_encountered(char* label)
{
	printf("label: %s\n",label);
	if (current_section == -1)
	{
		yyerror("define a section first");
	}
	if (find_label(label) != NULL)
	{
		yyerror("redifinition of label");
	}
	struct label *l = malloc(sizeof(struct label));
	if (l == NULL)
	{
		yyerror("Out of Memory!");
	}
	l->section = current_section;
	l->name = strdup(label);
	if (l->name == NULL)
	{
		yyerror("Out of Memory!");
	}
	l->offset = sections[current_section]->nextindex;
	add_label(l);
}

void section_encountered(char* section)
{
	printf("section: %s\n",section);
	if (find_section(section) == -1)
	{
		struct section *s = malloc(sizeof(struct section));
		if (s == NULL)
		{
			yyerror("Out of Memory!");
		}
		s->name = strdup(section);
		if (s->name == NULL)
		{
			yyerror("Out of Memory!");
		}
		s->offset = 0;
		s->data = NULL;
		s->size = 0;
		s->nextindex = 0;
		add_section(s);
		arm = true;
	}
	else
	{
		current_section = find_section(section);
	}
}

/*
function template:
void instruction_(char* inst,)
{
	enum condition c = get_condition(inst);
	if (arm)
	{
		
		
		yyerror("unsupported instruction");
	}
	else
	{
		yyerror("only arm instructions are currently supported");
	}
}
*/

/*



*/



void instruction_register_int(char* inst, int reg, int64_t int_val)
{
	if (arm)
	{
		if (strlen(inst) >= 3)
		{
			if (strncmp(inst,"mov",3) == 0)
			{
				enum condition c = get_condition(inst,3);
				uint32_t opcode = 0b00000011101000000000000000000000;
				opcode |= c << 28;
				opcode |= reg << 12;
				if (int_val > 0xff)
				{
					yyerror("invalid constant");
				}
				opcode |= int_val;
				
				section_write(current_section,&opcode,4,-1);
				return;
			}
		}
		yyerror("unsupported instruction");
	}
	else
	{
		yyerror("only arm instructions are currently supported");
	}
}

void instruction_register_register_register(char* inst, int reg1, int reg2, int reg3)
{
	if (arm)
	{
		if (strlen(inst) >= 3)
		{
			if (strncmp(inst,"add",3) == 0)
			{
				enum condition c = get_condition(inst,3);
				uint32_t opcode = 0b00000000100000000000000000000000;
				opcode |= c << 28;
				opcode |= reg1 << 12;
				opcode |= reg2 << 16;
				opcode |= reg3;
				
				section_write(current_section,&opcode,4,-1);
				return;
			}
		}
		yyerror("unsupported instruction");
	}
	else
	{
		yyerror("only arm instructions are currently supported");
	}
}

void instruction_register_register(char* inst, int reg1, int reg2)
{
	if (arm)
	{
		
		
		
		yyerror("unsupported instruction");
	}
	else
	{
		yyerror("only arm instructions are currently supported");
	}
}



void instruction_register_memory_register(char* inst, int reg1, int reg2)
{
	if (arm)
	{
		
		
		
		
		yyerror("unsupported instruction");
	}
	else
	{
		yyerror("only arm instructions are currently supported");
	}
}



void instruction_register_memory_label(char* inst, int reg, char* label)
{
	if (arm)
	{
		
		
		
		yyerror("unsupported instruction");
	}
	else
	{
		yyerror("only arm instructions are currently supported");
	}
}






















enum condition get_condition(char* str,int start)
{
	if (strlen(str+start) >= 2)
	{
		int last = start+1;
		if (str[last-1] == 'e' && str[last] == 'q')
		{
			return EQ;
		}
		if (str[last-1] == 'n' && str[last] == 'e')
		{
			return NE;
		}
		if (str[last-1] == 'c' && str[last] == 's')
		{
			return CSHS;
		}
		if (str[last-1] == 'c' && str[last] == 'c')
		{
			return CCLO;
		}
		if (str[last-1] == 'h' && str[last] == 's')
		{
			return CSHS;
		}
		if (str[last-1] == 'l' && str[last] == 'o')
		{
			return CCLO;
		}
		if (str[last-1] == 'm' && str[last] == 'i')
		{
			return MI;
		}
		if (str[last-1] == 'p' && str[last] == 'l')
		{
			return PL;
		}
		if (str[last-1] == 'v' && str[last] == 's')
		{
			return VS;
		}
		if (str[last-1] == 'v' && str[last] == 'c')
		{
			return VC;
		}
		if (str[last-1] == 'h' && str[last] == 'i')
		{
			return HI;
		}
		if (str[last-1] == 'l' && str[last] == 's')
		{
			return LS;
		}
		if (str[last-1] == 'g' && str[last] == 'e')
		{
			return GE;
		}
		if (str[last-1] == 'l' && str[last] == 't')
		{
			return LT;
		}
		if (str[last-1] == 'g' && str[last] == 't')
		{
			return GT;
		}
		if (str[last-1] == 'l' && str[last] == 'e')
		{
			return LE;
		}
	}
	return ALWAYS;
}






