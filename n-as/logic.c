#include "definitions.h"
#include <math.h>


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
	//printf("label: %s\n",label);
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
	//printf("section: %s\n",section);
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


int64_t string_to_immediate(char* str, int base)
{
	char* successful = NULL;
	long l = strtol(str,&successful,base);
	if (successful != NULL && *successful != '\0')
	{
		yyerror("could not convert to a 32 bit integer");
	}
	if (errno == ERANGE)
	{
		yyerror("could not convert to a 32 bit integer");
	}
	if (l < 0)
	{
		if (l < INT_MIN)
		{
			yyerror("could not convert to a 32 bit integer");
		}
	}
	else
	{
		if (l > INT_MAX)
		{
			yyerror("could not convert to a 32 bit integer");
		}
	}
	return (int32_t) l;
}


// checks if a number can be expressed as a rotated 8-bit number
// returns -1 if not possible
int is_rotated_imm(uint32_t num,uint8_t *rot_num)
{
	uint32_t rot = num;
	for (int i = 0;i<(1 << 4);i++)
	{
		if (rot == (rot & 0xff)) // if it fits into 8 bits
		{
			*rot_num = rot;
			return i;
		}
		rot = (rot << 2) | ((rot & (0b11 << 30)) >> 30);
	}
	return -1;
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


void assemble_mem_half_signed_imm(uint8_t l,uint8_t width,uint8_t flags, int64_t reg1, int64_t reg2, int64_t imm,enum addressing_mode addressing,uint8_t update_reg)
{
	if (arm)
	{
		uint8_t s = 0, h = 0;
		switch (width)
		{
			case 1:
				if (l == 1)
				{
					l = 0;
					s = 1;
					h = 0;
				}
				else
				{
					l = 0;
					s = 1;
					h = 1;
				}
				break;
			case 2:
				h = 1;
				s = 0;
				break;
			case 3:
				if (l == 0)
				{
					yyerror("sh is only supported for ldr instructions, use strh instead");
				}
				s = 1;
				h = 1;
				break;
			case 4:
				if (l == 0)
				{
					yyerror("bh is only supported for ldr instructions, use strb instead");
				}
				s = 1;
				break;
			default:
				yyerror("invalid width");
		}
		uint32_t write = 0;
		write |= flags << 28;
		write |= 1 << 22;
		write |= l << 20;
		write |= reg1 << 12;
		write |= reg2 << 16;
		write |= 1 << 7;
		write |= 1 << 4;
		write |= s << 6;
		write |= h << 5;
		if (imm < 0)
		{
			imm = - imm;
		}
		else
		{
			write |= 1 << 23;
		}
		if (imm >=  (2 << 7))
		{
			yyerror("offset is too big");
		}
		write |= (imm & 0b1111);
		write |= ((imm & 0b11110000) >> 4) << 8;
		switch (addressing)
		{
			case offset_addressing_mode:
				write |= 1 << 24;
				break;
			case pre_indexed_addressing_mode:
				write |= 1 << 24;
				break;
		}
		if (update_reg)
		{
			if (addressing == post_indexed_addressing_mode)
			{
				yyerror("the base register is always updated in post-indexed addressing");
			}
			write |= 1 << 21;
		}
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		yyerror("only arm instructions are currently supported");
	}
	yyerror("unsupported instruction");
}

void assemble_mem_half_signed_reg_offset(uint8_t l,uint8_t width,uint8_t flags, int64_t reg1, int64_t reg2, int64_t reg3,enum addressing_mode addressing,uint8_t update_reg,uint8_t u)
{
	if (arm)
	{
		uint8_t s = 0, h = 0;
		switch (width)
		{
			case 1:
				if (l == 1)
				{
					l = 0;
					s = 1;
					h = 0;
				}
				else
				{
					l = 0;
					s = 1;
					h = 1;
				}
				break;
			case 2:
				h = 1;
				s = 0;
				break;
			case 3:
				if (l == 0)
				{
					yyerror("sh is only supported for ldr instructions, use strh instead");
				}
				s = 1;
				h = 1;
				break;
			case 4:
				if (l == 0)
				{
					yyerror("bh is only supported for ldr instructions, use strb instead");
				}
				s = 1;
				break;
			default:
				yyerror("invalid width");
		}
		uint32_t write = 0;
		write |= flags << 28;
		write |= l << 20;
		write |= reg1 << 12;
		write |= reg2 << 16;
		write |= 1 << 7;
		write |= 1 << 4;
		write |= s << 6;
		write |= h << 5;
		write |= reg3;
		write |= u << 23;
		
		switch (addressing)
		{
			case offset_addressing_mode:
				write |= 1 << 24;
				break;
			case pre_indexed_addressing_mode:
				write |= 1 << 24;
				break;
		}
		if (update_reg)
		{
			if (addressing == post_indexed_addressing_mode)
			{
				yyerror("the base register is always updated in post-indexed addressing");
			}
			write |= 1 << 21;
		}
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		yyerror("only arm instructions are currently supported");
	}
	yyerror("unsupported instruction");
}








void assemble_mem_word_ubyte_imm(uint8_t l,uint8_t b, uint8_t t,uint8_t flags, int64_t reg1, int64_t reg2, int64_t imm,enum addressing_mode addressing,uint8_t update_reg)
{
	if (arm)
	{
		uint32_t write = 0;
		write |= flags << 28;
		write |= 1 << 26;
		write |= l << 20;
		write |= b << 22;
		write |= reg1 << 12;
		write |= reg2 << 16;
		if (imm < 0)
		{
			imm = - imm;
		}
		else
		{
			write |= 1 << 23;
		}
		if (imm >=  (2 << 11))
		{
			yyerror("offset is too big");
		}
		write |= imm;
		if (t == 0)
		{
			switch (addressing)
			{
				case offset_addressing_mode:
					write |= 1 << 24;
					break;
				case pre_indexed_addressing_mode:
					write |= 1 << 24;
					break;
			}
			if (update_reg)
			{
				if (addressing == post_indexed_addressing_mode)
				{
					yyerror("the base register is always updated in post-indexed addressing");
				}
				write |= 1 << 21;
			}
		}
		else
		{
			if (addressing != post_indexed_addressing_mode)
			{
				yyerror("*t instructions only work with post-indexed access");
			}
			write |= 1 << 21;
		}
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		yyerror("only arm instructions are currently supported");
	}
	yyerror("unsupported instruction");
}

void assemble_mem_word_ubyte_reg_offset(uint8_t l,uint8_t b, uint8_t t,uint8_t flags, int64_t reg1, int64_t reg2, int64_t reg3,enum addressing_mode addressing,uint8_t update_reg,uint8_t u)
{
	if (arm)
	{
		uint32_t write = 0;
		write |= flags << 28;
		write |= 1 << 26;
		write |= l << 20;
		write |= b << 22;
		write |= reg1 << 12;
		write |= reg2 << 16;
		write |= reg3;
		write |= u << 23;
		write |= 1 << 25;
		if (t == 0)
		{
			switch (addressing)
			{
				case offset_addressing_mode:
					write |= 1 << 24;
					break;
				case pre_indexed_addressing_mode:
					write |= 1 << 24;
					break;
			}
			if (update_reg)
			{
				if (addressing == post_indexed_addressing_mode)
				{
					yyerror("the base register is always updated in post-indexed addressing");
				}
				write |= 1 << 21;
			}
		}
		else
		{
			if (addressing != post_indexed_addressing_mode)
			{
				yyerror("*t instructions only work with post-indexed access");
			}
			write |= 1 << 21;
		}
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		yyerror("only arm instructions are currently supported");
	}
	yyerror("unsupported instruction");
}
void assemble_mem_word_ubyte_reg_offset_scaled(uint8_t l,uint8_t b, uint8_t t,uint8_t flags, int64_t reg1, int64_t reg2, int64_t reg3,uint8_t shift_type, int64_t shift_val,enum addressing_mode addressing,uint8_t update_reg,uint8_t u)
{
	if (arm)
	{
		uint32_t write = 0;
		write |= flags << 28;
		write |= 1 << 26;
		write |= l << 20;
		write |= b << 22;
		write |= reg1 << 12;
		write |= reg2 << 16;
		write |= reg3;
		write |= u << 23;
		write |= 1 << 25;
		if (shift_val >= 0b100000)
		{
			yyerror("immediate shift is too big");
		}
		if (shift_type == 0b11 && shift_val == 0)
		{
			yyerror("ror #0 is not supported");
		}
		if (shift_type == 0b111)
		{
			shift_type = 0b11;
			shift_val = 0;
		}
		write |= shift_val << 7;
		write |= shift_type << 5;
		if (t == 0)
		{
			switch (addressing)
			{
				case offset_addressing_mode:
					write |= 1 << 24;
					break;
				case pre_indexed_addressing_mode:
					write |= 1 << 24;
					break;
			}
			if (update_reg)
			{
				if (addressing == post_indexed_addressing_mode)
				{
					yyerror("the base register is always updated in post-indexed addressing");
				}
				write |= 1 << 21;
			}
		}
		else
		{
			if (addressing != post_indexed_addressing_mode)
			{
				yyerror("*t instructions only work with post-indexed access");
			}
			write |= 1 << 21;
		}
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		yyerror("only arm instructions are currently supported");
	}
	yyerror("unsupported instruction");
}





void assemble_comp_reg_imm(uint32_t opcode,uint8_t flags,int64_t reg,int64_t imm)
{
	if (arm)
	{
		uint32_t write = 0;
		write |= flags << 28;
		write |= 1 << 25; // immediate form
		write |= opcode << 21;
		write |= reg << 16;
		write |= 1 << 20; // always update the flags
		
		uint8_t rot_imm = 0;
		int rot = is_rotated_imm(imm,&rot_imm);
		if (rot == -1)
		{
			yyerror("immediate value cannot be rotated into a 8 bit constant");
		}
		write |= rot_imm;
		write |= rot << 8;
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		yyerror("only arm instructions are currently supported");
	}
	yyerror("unsupported instruction");
}


void assemble_comp_reg_reg(uint32_t opcode,uint8_t flags,int64_t reg1,int64_t reg2)
{
	assemble_comp_reg_reg_shift(opcode,flags,reg1,reg2,0,0);
}

void assemble_comp_reg_reg_shift(uint32_t opcode,uint8_t flags,int64_t reg1,int64_t reg2,uint8_t shift_type,uint8_t shift_val)
{
	if (arm)
	{
		uint32_t write = 0;
		write |= flags << 28;
		write |= opcode << 21;
		write |= reg1 << 16;
		write |= reg2;
		write |= 1 << 20; // always update the flags
		if (shift_val >= 0b100000)
		{
			yyerror("immediate shift is too big");
		}
		if (shift_type == 0b111)
		{
			yyerror("rrx is not supported for immediate shifts");
		}
		if (shift_type == 0b11 && shift_val == 0)
		{
			yyerror("ror #0 is not supported");
		}
		write |= shift_val << 7;
		write |= shift_type << 5;
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		yyerror("only arm instructions are currently supported");
	}
	yyerror("unsupported instruction");
}

void assemble_comp_reg_reg_shift_reg(uint32_t opcode,uint8_t flags,int64_t reg1,int64_t reg2,uint8_t shift_type,uint8_t shift_reg)
{
	if (arm)
	{
		uint32_t write = 0;
		write |= flags << 28;
		write |= opcode << 21;
		write |= reg1 << 16;
		write |= reg2;
		write |= 1 << 20; // always update the flags
		write |= 1 << 4;
		if (shift_type == 0b111)
		{
			shift_type = 0b11;
			shift_reg = 0;
			write &= ~ (1 << 4);
		}
		write |= shift_reg << 8;
		write |= shift_type << 5;
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		yyerror("only arm instructions are currently supported");
	}
	yyerror("unsupported instruction");
}



void assemble_data_proc_reg_imm(uint32_t opcode,uint8_t flags,uint8_t update_flags,int64_t reg,int64_t imm)
{
	if (arm)
	{
		uint32_t write = 0;
		write |= flags << 28;
		write |= 1 << 25; // immediate form
		write |= opcode << 21;
		write |= update_flags << 20;
		write |= reg << 12;
		
		uint8_t rot_imm = 0;
		int rot = is_rotated_imm(imm,&rot_imm);
		if (rot == -1)
		{
			yyerror("immediate value cannot be rotated into a 8 bit constant");
		}
		write |= rot_imm;
		write |= rot << 8;
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		yyerror("only arm instructions are currently supported");
	}
	yyerror("unsupported instruction");
}


void assemble_data_proc_reg_reg(uint32_t opcode,uint8_t flags,uint8_t update_flags,int64_t reg1,int64_t reg2)
{
	if (arm)
	{
		uint32_t write = 0;
		write |= flags << 28;
		write |= opcode << 21;
		write |= update_flags << 20;
		write |= reg1 << 12;
		write |= reg2;
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		yyerror("only arm instructions are currently supported");
	}
	yyerror("unsupported instruction");
}

void assemble_data_proc_reg_reg_shift(uint32_t opcode,uint8_t flags,uint8_t update_flags,int64_t reg1,int64_t reg2,uint8_t shift_type,uint8_t shift_val)
{
	if (arm)
	{
		uint32_t write = 0;
		write |= flags << 28;
		write |= opcode << 21;
		write |= update_flags << 20;
		write |= reg1 << 12;
		write |= reg2;
		if (shift_val >= 0b100000)
		{
			yyerror("immediate shift is too big");
		}
		if (shift_type == 0b111)
		{
			yyerror("rrx is not supported for immediate shifts");
		}
		if (shift_type == 0b11 && shift_val == 0)
		{
			yyerror("ror #0 is not supported");
		}
		write |= shift_val << 7;
		write |= shift_type << 5;
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		yyerror("only arm instructions are currently supported");
	}
	yyerror("unsupported instruction");
}

void assemble_data_proc_reg_reg_shift_reg(uint32_t opcode,uint8_t flags,uint8_t update_flags,int64_t reg1,int64_t reg2,uint8_t shift_type,uint8_t shift_reg)
{
	if (arm)
	{
		uint32_t write = 0;
		write |= flags << 28;
		write |= opcode << 21;
		write |= update_flags << 20;
		write |= reg1 << 12;
		write |= reg2;
		write |= 1 << 4;
		if (shift_type == 0b111)
		{
			shift_type = 0b11;
			shift_reg = 0;
			write &= ~ (1 << 4);
		}
		write |= shift_reg << 8;
		write |= shift_type << 5;
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		yyerror("only arm instructions are currently supported");
	}
	yyerror("unsupported instruction");
}


void assemble_data_proc_reg_reg_imm(uint32_t opcode,uint8_t flags,uint8_t update_flags,int64_t reg1, int64_t reg2,int64_t imm)
{
	if (arm)
	{
		uint32_t write = 0;
		write |= flags << 28;
		write |= 1 << 25; // immediate form
		write |= opcode << 21;
		write |= update_flags << 20;
		write |= reg1 << 12;
		write |= reg2 << 16;
		
		
		uint8_t rot_imm = 0;
		int rot = is_rotated_imm(imm,&rot_imm);
		if (rot == -1)
		{
			yyerror("immediate value cannot be rotated into a 8 bit constant");
		}
		write |= rot_imm;
		write |= rot << 8;
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		yyerror("only arm instructions are currently supported");
	}
	yyerror("unsupported instruction");
}


void assemble_data_proc_reg_reg_reg(uint32_t opcode,uint8_t flags,uint8_t update_flags,int64_t reg1,int64_t reg2,int64_t reg3) // this is a special case of a register with immediate shift, which is 0 in this case
{
	assemble_data_proc_reg_reg_reg_shift(opcode,flags,update_flags,reg1,reg2,reg3,0,0);
}


void assemble_data_proc_reg_reg_reg_shift(uint32_t opcode,uint8_t flags,uint8_t update_flags,int64_t reg1,int64_t reg2,int64_t reg3,uint8_t shift_type,uint8_t shift_val)
{
	if (arm)
	{
		uint32_t write = 0;
		write |= flags << 28;
		write |= opcode << 21;
		write |= update_flags << 20;
		write |= reg1 << 12;
		write |= reg2 << 16;
		write |= reg3;
		if (shift_val >= 0b100000)
		{
			yyerror("immediate shift is too big");
		}
		if (shift_type == 0b111)
		{
			yyerror("rrx is not supported for immediate shifts");
		}
		if (shift_type == 0b11 && shift_val == 0)
		{
			yyerror("ror #0 is not supported");
		}
		write |= shift_val << 7;
		write |= shift_type << 5;
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		yyerror("only arm instructions are currently supported");
	}
	yyerror("unsupported instruction");
}

void assemble_data_proc_reg_reg_reg_shift_reg(uint32_t opcode,uint8_t flags,uint8_t update_flags,int64_t reg1,int64_t reg2,int64_t reg3,uint8_t shift_type,uint8_t shift_reg)
{
	if (arm)
	{
		uint32_t write = 0;
		write |= flags << 28;
		write |= opcode << 21;
		write |= update_flags << 20;
		write |= reg1 << 12;
		write |= reg2 << 16;
		write |= reg3;
		write |= 1 << 4;
		if (shift_type == 0b111)
		{
			shift_type = 0b11;
			shift_reg = 0;
			write &= ~ (1 << 4);
		}
		write |= shift_reg << 8;
		write |= shift_type << 5;
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		yyerror("only arm instructions are currently supported");
	}
	yyerror("unsupported instruction");
}



/*
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
		if (strlen(inst) >= 3)
		{
			if (strncmp(inst,"mov",3) == 0)
			{
				enum condition c = get_condition(inst,3);
				uint32_t opcode = 0b00000001101000000000000000000000;
				opcode |= c << 28;
				opcode |= reg1 << 12;
				opcode |= reg2;
				
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



void instruction_register_memory_register(char* inst, int reg1, int reg2,int64_t offset) // if offset is 0, the register value alone is used
{
	if (arm)
	{
		if (strlen(inst) >= 3)
		{
			if (strncmp(inst,"ldr",3) == 0)
			{
				enum condition c = get_condition(inst,3);
				if (c == ALWAYS && strlen(inst) > 3) // if it is not a conditional, it is a size specifier
				{
					yyerror("unsupported instruction");
				}
				uint32_t opcode = 0b00000100000100000000000000000000;
				opcode |= c << 28;
				opcode |= reg1 << 12;
				opcode |= reg2 << 16;
				if (offset >= pow(2,12) || - offset >= pow(2,12))
				{
					yyerror("offset too big");
				}
				if (offset > 0)
				{
					opcode |= offset;
					opcode |= 1 << 23;
				}
				else
				{
					opcode |= - offset;
				}
				// 1 << 21 is the t-bit. using it makes the instruction use user-mode level of page table access, so a acces to privileged-mode memory generates a data abort
				//opcode |= 1 << 21;
				
				
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
*/


// frees all labels, fixups and sections
void free_data()
{
	for (uint32_t i = 0;i<sections_size;i++)
	{
		if (sections[i] != NULL)
		{
			free(sections[i]->data);
			sections[i]-> data = NULL;
			sections[i]->size = 0;
			free(sections[i]->name);
			sections[i]->name = NULL;
			free(sections[i]);
			sections[i] = NULL;
		}
	}
	for (uint32_t i = 0;i<labels_size;i++)
	{
		if (labels[i] != NULL)
		{
			free(labels[i]->name);
			labels[i]->name = NULL;
			free(labels[i]);
			labels[i] = NULL;
		}
	}
	for (uint32_t i = 0;i<fixups_size;i++)
	{
		if (fixups[i] != NULL)
		{
			free(fixups[i]->name);
			fixups[i]->name = NULL;
			free(fixups[i]);
			fixups[i] = NULL;
		}
	}
}


/*
void* __real_malloc(size_t);
void __real_free(void*);

uint64_t allocations = 0;
void* __wrap_malloc(size_t size)
{
	allocations++;
	return __real_malloc(size);
}
void __wrap_free(void* p)
{
	allocations--;
	__real_free(p);
}
*/








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






