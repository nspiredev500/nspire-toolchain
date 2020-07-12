#include "definitions.h"
#include <math.h>



uint16_t assembler_flags = 0;

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

void section_read(void* dest,int sect, int size,int offset)
{
	if (sect < 0 || sect >= sections_size)
	{
		printf("define a section first\n");
		return;
	}
	struct section* s = sections[sect];
	if (s == NULL)
	{
		return;
	}
	if (offset < 0)
	{
		printf("offset has to be > 0\n");
		return;
	}
	char* d = dest;
	if (s->data == NULL)
	{
		memset(dest,0,size);
		return;
	}
	char* so = s->data;
	for (int i = 0;i<size;i++)
	{
		if (offset+i < s->size)
		{
			d[i] = so[offset+i];
		}
		else
		{
			d[i] = 0;
		}
	}
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

void label_defined(char* label)
{
	//printf("label: %s\n",label);
	if (find_label(label) != NULL)
	{
		return;
	}
	struct label *l = malloc(sizeof(struct label));
	if (l == NULL)
	{
		yyerror("Out of Memory!");
		return;
	}
	l->section = -1;
	l->name = strdup(label);
	if (l->name == NULL)
	{
		yyerror("Out of Memory!");
		return;
	}
	l->offset = -1;
	add_label(l);
}

void label_encountered(char* label)
{
	//printf("label: %s\n",label);
	if (current_section == -1)
	{
		yyerror("define a section first");
		return;
	}
	{
		struct label *found = find_label(label);
		if (found != NULL)
		{
			if (found->section == -1) // label already defined, now it's found
			{
				found->section = current_section;
				found->offset = sections[current_section]->nextindex;
				return;
			}
			yyerror("redifinition of label");
			return;
		}
	}
	struct label *l = malloc(sizeof(struct label));
	if (l == NULL)
	{
		yyerror("Out of Memory!");
		return;
	}
	l->section = current_section;
	l->name = strdup(label);
	if (l->name == NULL)
	{
		yyerror("Out of Memory!");
		return;
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
			return;
		}
		s->name = strdup(section);
		if (s->name == NULL)
		{
			yyerror("Out of Memory!");
			return;
		}
		s->offset = 0;
		s->data = NULL;
		s->size = 0;
		s->nextindex = 0;
		add_section(s);
		arm = true;
		current_section = find_section(section);
	}
	else
	{
		current_section = find_section(section);
	}
}


// returns the size of the full binary
int arrange_sections()
{
	if (sections != NULL)
	{
		int size = 0;
		for (int i = 0;i<sections_size;i++)
		{
			if (sections[i] != NULL)
			{
				sections[i]->offset = size;
				size += sections[i]->nextindex;
				if (size % 4 != 0)
				{
					size += size % 4; // add the remainder to make it aligned with 4 bytes again, as this is also the base for the next section
				}
			}
		}
		return size;
	}
	else
	{
		return 0;
	}
}

void assemble_binary(void* binary,int max)
{
	if (sections != NULL)
	{
		int size = 0;
		for (int i = 0;i<sections_size;i++)
		{
			if (sections[i] != NULL)
			{
				if (sections[i]->data == NULL)
					continue;
				if (size+sections[i]->nextindex < max)
				{
					memcpy(binary+size,sections[i]->data,sections[i]->nextindex);
				}
				else
				{
					memcpy(binary+size,sections[i]->data,max-size);
					return;
				}
				size += sections[i]->nextindex;
				if (size % 4 != 0)
				{
					size += size % 4; // add the remainder to make it aligned with 4 bytes again, as this is also the base for the next section
				}
			}
		}
		return;
	}
	else
	{
		return;
	}
}

bool apply_fixups()
{
	if	(fixups != NULL)
	{
		for (int i = 0;i<fixups_size;i++)
		{
			if (fixups[i] != NULL)
			{
				struct label *l = find_label(fixups[i]->name);
				if (l == NULL || l->section == -1)
				{
					printf("label not found: %s\n",fixups[i]->name);
					return false;
				}
				int label_offset = sections[l->section]->offset + l->offset; // offset into the binary
				int fixup_offset = sections[fixups[i]->section]->offset + fixups[i]->offset;
				int32_t diff =  label_offset - fixup_offset;
				uint32_t write = 0;
				switch (fixups[i]->fixup_type)
				{
				case FIXUP_B:
					printf("diff: %d\n",diff);
					diff -= 8;
					if (diff % 4 != 0)
					{
						printf("branch offset has to be a multiple of 4\n");
						return false;
					}
					bool minus = false;
					if (diff < 0)
					{
						minus = true;
						diff = - diff;
					}
					diff = diff >> 2;
					if (diff >= (2 << 22)) // bit 23 is the sign bit
					{
						printf("branch offset is too big\n");
						return false;
					}
					if (minus)
					{
						diff = (-diff) & 0xffffff;
					}
					section_write(fixups[i]->section,&diff,3,fixups[i]->offset);
					break;
				case FIXUP_BLX:
					diff -= 8;
					minus = false;
					if (diff < 0)
					{
						minus = true;
						diff = - diff;
					}
					if (diff % 2 != 0)
					{
						printf("branch offset has to be a multiple of 2\n");
						return false;
					}
					uint8_t h = 0;
					if (diff & 0b10 != 0)
					{
						h |= 1;
					}
					diff = diff >> 2;
					if (diff >= (2 << 22))
					{
						printf("branch offset is too big\n");
						return false;
					}
					if (minus)
					{
						diff = (-diff) & 0xffffff;
					}
					diff |= 0b1111101 << 25; // just overwrite the rest too, we have to change the first bit of the fourth byte
					diff |= h << 24;;
					section_write(fixups[i]->section,&diff,4,fixups[i]->offset);
					break;
				case FIXUP_MEM_W_B:
					write = 0;
					diff -= 8;
					section_read(&write,fixups[i]->section,4,fixups[i]->offset);
					write &= ~0xfff; // clear the first 12 bits for the offset
					write &= ~ (1 << 23); // clear the positive bit
					if (diff < 0)
					{
						diff = - diff;
					}
					else
					{
						write |= 1 << 23;
					}
					if (diff >=  (2 << 11))
					{
						printf("offset is too big");
						return false;
					}
					write |= diff;
					section_write(fixups[i]->section,&write,4,fixups[i]->offset);
					break;
				case FIXUP_MEM_H:
					write = 0;
					diff -= 8;
					section_read(&write,fixups[i]->section,4,fixups[i]->offset);
					write &= ~ 0b1111;
					write &= ~ (0b1111 << 8);
					write &= ~ (1 << 23); // clear the positive bit
					if (diff < 0)
					{
						diff = - diff;
					}
					else
					{
						write |= 1 << 23;
					}
					if (diff >=  (2 << 7))
					{
						printf("offset is too big\n");
						return false;
					}
					write |= (diff & 0b1111);
					write |= ((diff & 0b11110000) >> 4) << 8;
					section_write(fixups[i]->section,&write,4,fixups[i]->offset);
					break;
				default:
					printf("invalid fixup type!\n");
					return false;
				}
			}
		}
	}
	return true;
}


int64_t string_to_immediate(char* str, int base)
{
	char* successful = NULL;
	long l = strtol(str,&successful,base);
	if (l == 0 && errno != 0)
	{
		yyerror("could not convert to a 32 bit integer");
		return 0xffffffffff; // bigger than any 32 bit number, so this works
	}
	if (l < 0)
	{
		if (l < -pow(2,32)) // will be used as uint32_t anyways
		{
			yyerror("could not convert to a 32 bit integer");
			return 0xffffffffff;
		}
	}
	else
	{
		if (l > pow(2,32)) // will be used as uint32_t anyways
		{
			yyerror("could not convert to a 32 bit integer");
			return 0xffffffffff;
		}
	}
	return (int64_t) l;
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



uint16_t register_range(int64_t r1, int64_t r2)
{
	if (r2 < r1)
	{
		uint64_t tmp = r2;
		r2 = r1;
		r1 = tmp;
	}
	uint16_t reglist = 0;
	for (int i = r1;i<=r2;i++) // <= because the register list is inclusive
	{
		reglist |= 1 << i;
	}
	return reglist;
}


void assemble_msr_imm(uint8_t flags,uint8_t spsr,uint8_t psr_fields, int64_t imm)
{
	/* psr_fields:
	c:
    control field mask byte, PSR[7:0] (privileged software execution) 
	x:
    extension field mask byte, PSR[15:8] (privileged software execution) 
	s:
    status field mask byte, PSR[23:16] (privileged software execution) 
	f:
    flags field mask byte, PSR[31:24] (privileged software execution).
	*/
	if ((assembler_flags & ASSEMBLER_PSR_ALLOWED) == 0)
	{
		yyerror("msr and mrs are not allowed");
		return;
	}
	if (arm)
	{
		uint32_t write = 0;
		write |= flags << 28;
		write |= 0b11 << 24;
		write |= spsr << 22;
		write |= 1 << 21;
		write |= psr_fields << 16;
		write |= 0b1111 << 12;
		
		
		if (imm < 0)
		{
			yyerror("immediate value can't be 0");
			return;
		}
		uint8_t rot_imm = 0;
		int rot = is_rotated_imm(imm,&rot_imm);
		if (rot == -1)
		{
			yyerror("immediate value cannot be rotated into a 8 bit constant");
			return;
		}
		write |= rot_imm;
		write |= rot << 8;
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		yyerror("only arm instructions are currently supported");
		return;
	}
	yyerror("unsupported instruction");
}

void assemble_msr_reg(uint8_t flags,uint8_t spsr,uint8_t psr_fields, int64_t reg)
{
	if ((assembler_flags & ASSEMBLER_PSR_ALLOWED) == 0)
	{
		yyerror("msr and mrs are not allowed");
		return;
	}
	if (arm)
	{
		uint32_t write = 0;
		write |= flags << 28;
		write |= 1 << 24;
		write |= spsr << 22;
		write |= 1 << 21;
		write |= psr_fields << 16;
		write |= 0b1111 << 12;
		write |= reg;
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		yyerror("only arm instructions are currently supported");
		return;
	}
	yyerror("unsupported instruction");
}

void assemble_mrs(uint8_t flags, int64_t reg,uint8_t spsr)
{
	if ((assembler_flags & ASSEMBLER_PSR_ALLOWED) == 0)
	{
		yyerror("msr and mrs are not allowed");
		return;
	}
	if (arm)
	{
		uint32_t write = 0;
		write |= flags << 28;
		write |= 1 << 24;
		write |= spsr << 22;
		write |= 0b1111 << 16;
		write |= reg << 12;
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		yyerror("only arm instructions are currently supported");
		return;
	}
	yyerror("unsupported instruction");
}



void assemble_blx_reg(uint8_t flags, int64_t reg)
{
	if (arm)
	{
		uint32_t write = 0;
		write |= flags << 28;
		write |= 0b1001 << 21;
		write |= 0b111111111111 << 8;
		write |= 0b11 << 4;
		write |= reg;
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		yyerror("only arm instructions are currently supported");
		return;
	}
	yyerror("unsupported instruction");
}

void assemble_blx_label(char* label)
{
	if (arm)
	{
		uint32_t write = 0;
		write |= 0b1111101 << 25;;
		
		
		if (current_section == -1)
		{
			yyerror("define a section first");
			return;
		}
		struct fixup *f = malloc(sizeof(struct fixup));
		if (f == NULL)
		{
			yyerror("Out of Memory!");
			return;
		}
		f->name = strdup(label);
		f->offset = sections[current_section]->nextindex;
		f->section = current_section;
		f->fixup_type = FIXUP_BLX;
		add_fixup(f);
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		yyerror("only arm instructions are currently supported");
		return;
	}
	yyerror("unsupported instruction");
}


void assemble_blx_imm(int64_t imm)
{
	if (arm)
	{
		uint32_t write = 0;
		write |= 0b1111101 << 25;;
		bool minus = false;
		if (imm < 0)
		{
			minus = true;
			imm = - imm;
		}
		if (imm % 2 != 0)
		{
			yyerror("branch offset has to be a multiple of 2");
			return;
		}
		if (imm & 0b10 != 0)
		{
			write |= 1 << 24;
		}
		imm = imm >> 2;
		if (imm >= (2 << 22))
		{
			yyerror("branch offset is too big");
			return;
		}
		if (minus)
		{
			imm = (-imm) & 0xffffff;
		}
		write |= imm;
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		yyerror("only arm instructions are currently supported");
		return;
	}
	yyerror("unsupported instruction");
}

void assemble_bx(uint8_t flags, int64_t reg)
{
	if (arm)
	{
		uint32_t write = 0;
		write |= flags << 28;
		write |= 0b1001 << 21;
		write |= 0b111111111111 << 8;
		write |= 1 << 4;
		write |= reg;
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		yyerror("only arm instructions are currently supported");
		return;
	}
	yyerror("unsupported instruction");
}
























void assemble_mul(uint8_t inst,uint8_t mul_width,uint8_t update_flags, uint8_t flags,int64_t reg1,int64_t reg2,int64_t reg3,int64_t reg4)
{
	if (arm)
	{
		uint32_t write = 0;
		write |= flags << 28;
		if (inst < 4)
		{
			if (mul_width != 0)
			{
				yyerror("this multiplication instruction doesn't need a width specifier");
				return;
			}
			write |= 0b1001 << 4;
			write |= update_flags << 20;
			if (inst > 1)
			{
				if (reg4 == -1)
				{
					yyerror("not enough registers");
					return;
				}
				write |= reg1 << 12;
				write |= reg2 << 16;
				write |= reg3;
				write |= reg4 << 8;
				if (inst == 3)
				{
					write |= 1 << 23;
				}
				else
				{
					write |= 0b11 << 22;
				}
			}
			else
			{
				if (inst == 0)
				{
					write |= reg1 << 16;
					write |= reg2;
					write |= reg3 << 8;
				}
				else
				{
					if (reg4 == -1)
					{
						yyerror("not enough registers");
						return;
					}
					write |= reg1 << 16;
					write |= reg2;
					write |= reg3 << 8;
					write |= reg4 << 12;
					write |= 1 << 21;
				}
			}
		}
		else
		{
			if (inst == 4 || inst == 5 || inst == 7)
			{
				if (mul_width == 0)
				{
					if (inst != 4)
					{
						yyerror("instruction needs b/t");
						return;
					}
					write |= update_flags << 20;
					write |= 0b1001 << 4;
					write |= 0b111 << 21;
					write |= reg1 << 12;
					write |= reg2 << 16;
					write |= reg3;
					write |= reg4 << 8;
				}
				else
				{
					if (update_flags != 0)
					{
						yyerror("s suffix not allowed for this instruction");
						return;
					}
					switch (inst)
					{
						case 4:
							write |= 0b101 << 22;
							if (mul_width < 3)
							{
								yyerror("invalid multiply suffix");
								return;
							}
							break;
						case 5:
							write |= 1 << 24;
							if (mul_width < 3)
							{
								yyerror("invalid multiply suffix");
								return;
							}
							break;
						case 7:
							write |= 0b1011 << 21;
							if (mul_width < 3)
							{
								yyerror("invalid multiply suffix");
								return;
							}
							break;
					}
					switch(inst)
					{
						case 4:
							write |= reg1 << 12;
							write |= reg2 << 16;
							write |= reg3;
							write |= reg4 << 8;
							if (reg4 == -1)
							{
								yyerror("this instruction needs 4 operands");
								return;
							}
							break;
						case 5:
							write |= reg1 << 16;
							write |= reg2;
							write |= reg3 << 8;
							write |= reg4 << 12;
							if (reg4 == -1)
							{
								yyerror("this instruction needs 4 operands");
								return;
							}
							break;
						case 7:
							write |= reg1 << 16;
							write |= reg2;
							write |= reg3 << 8;
							if (reg4 != -1)
							{
								yyerror("this instruction only needs 3 operands");
								return;
							}
							break;
					}
					write |= 1 << 7;
					switch (mul_width)
					{
						case 3:
							
							break;
						case 4:
							write |= 1 << 6;
							break;
						case 5:
							write |= 1 << 5;
							break;
						case 6:
							write |= 0b11 << 5;
							break;
					}
				}
				section_write(current_section,&write,4,-1);
				return;
			}
			if (inst == 6 || inst == 8)
			{
				if (mul_width != 1 && mul_width != 2)
				{
					yyerror("invalid multiply suffix");
					return;
				}
				if (mul_width == 2)
				{
					write |= 1 << 6;
				}
				if (update_flags != 0)
				{
					yyerror("s suffix not allowed for this instruction");
					return;
				}
				write |= 1 << 7;
				write |= 0b1001 << 21;
				if (inst == 6 && reg4 != -1)
				{
					yyerror("this instruction only needs 3 operands");
					return;
				}
				if (inst == 8 && reg4 == -1)
				{
					yyerror("this instruction needs 4 operands");
					return;
				}
				write |= reg1 << 16;
				write |= reg2;
				write |= reg3 << 8;
				if (inst == 8)
				{
					write |= reg4 << 12;
				}
				else
				{
					write |= 1 << 5;
				}
				section_write(current_section,&write,4,-1);
				return;
			}
			yyerror("unsupported instruction");
			return;
		}
		
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		yyerror("only arm instructions are currently supported");
		return;
	}
	yyerror("unsupported instruction");
}



void assemble_swp(uint8_t b,uint8_t flags,int64_t reg1,int64_t reg2,int64_t reg3)
{
	if (arm)
	{
		uint32_t write = 0;
		write |= flags << 28;
		write |= 1 << 24;
		write |= 1 << 7;
		write |= 1 << 4;
		write |= b << 22;
		write |= reg1 << 12;
		write |= reg2;
		write |= reg3 << 16;
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		yyerror("only arm instructions are currently supported");
		return;
	}
	yyerror("unsupported instruction");
}


void assemble_coproc(uint8_t mrc,uint8_t flags,int64_t coproc,int64_t opcode1,int64_t reg,int64_t coproc_reg1,int64_t coproc_reg2,int64_t opcode2)
{
	if ((assembler_flags & ASSEMBLER_COPROCESSOR_ALLOWED) == 0)
	{
		yyerror("coprocessor instructions are disabled");
		return;
	}
	if (arm)
	{
		uint32_t write = 0;
		if (coproc > 15)
		{
			yyerror("invalid coprocessor number");
			return;
		}
		if (opcode1 >= (1 << 3))
		{
			yyerror("invalid opcode 1");
			return;
		}
		if (opcode2 >= (1 << 3))
		{
			yyerror("invalid opcode 2");
			return;
		}
		if (coproc_reg1 >= (1 << 4))
		{
			yyerror("invalid coprocessor register");
			return;
		}
		if (coproc_reg2 >= (1 << 4))
		{
			yyerror("invalid coprocessor register");
			return;
		}
		write |= flags << 28;
		write |= 0b111 << 25;
		write |= 1 << 4;
		write |= mrc << 20;
		write |= opcode1 << 21;
		write |= opcode2 << 5;
		write |= coproc << 8;
		write |= coproc_reg1 << 16;
		write |= coproc_reg2;
		write |= reg << 12;
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		yyerror("only arm instructions are currently supported");
		return;
	}
	yyerror("unsupported instruction");
}





void assemble_swi(uint8_t flags, int64_t imm)
{
	if ((assembler_flags & ASSEMBLER_SWI_ALLOWED) == 0)
	{
		yyerror("swi instructions are disabled");
		return;
	}
	if (arm)
	{
		uint32_t write = 0;
		write |= flags << 28;
		write |= 0b1111 << 24;
		if (imm < 0)
		{
			yyerror("swi numbers are always positive");
			return;
		}
		if (imm >= (1 << 25))
		{
			yyerror("swi number too big!");
			return;
		}
		write |= imm;
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		yyerror("only arm instructions are currently supported");
		return;
	}
	yyerror("unsupported instruction");
}


void assemble_clz(uint8_t flags,int64_t reg1, int64_t reg2)
{
	if (arm)
	{
		uint32_t write = 0;
		write |= flags << 28;
		write |= 1 << 24;
		write |= 0b11 << 21;
		write |= 0b1111 << 16;
		write |= 0b1111 << 8;
		write |= 1 << 4;
		write |= reg1 << 12;
		write |= reg2;
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		yyerror("only arm instructions are currently supported");
		return;
	}
	yyerror("unsupported instruction");
}




void assemble_mem_multiple(uint8_t l,uint8_t adr_mode,uint8_t flags, int64_t reg1,uint8_t update_reg,uint16_t reglist,uint8_t user_mode_regs)
{
	if (arm)
	{
		uint32_t write = 0;
		write |= flags << 28;
		write |= 1 << 27;
		write |= user_mode_regs << 22;
		write |= update_reg << 21;
		write |= l << 20;
		switch (adr_mode)
		{
			case 0:
				write |= 1 << 23;
				break;
			case 1:
				write |= 1 << 23;
				write |= 1 << 24;
				break;
			case 2:
				
				break;
			case 3:
				write |= 1 << 24;
				break;
		}
		write |= reglist;
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		yyerror("only arm instructions are currently supported");
		return;
	}
	yyerror("unsupported instruction");
}




void assemble_branch_label(uint8_t l,uint8_t flags,char* label)
{
	if (arm)
	{
		uint32_t write = 0;
		write |= flags << 28;
		write |= 1 << 27;
		write |= 1 << 25;
		write |= l << 24;
		
		
		
		if (current_section == -1)
		{
			yyerror("define a section first");
			return;
		}
		struct fixup *f = malloc(sizeof(struct fixup));
		if (f == NULL)
		{
			yyerror("Out of Memory!");
			return;
		}
		f->name = strdup(label);
		f->offset = sections[current_section]->nextindex;
		f->section = current_section;
		f->fixup_type = FIXUP_B;
		add_fixup(f);
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		yyerror("only arm instructions are currently supported");
		return;
	}
	yyerror("unsupported instruction");
}

void assemble_branch(uint8_t l,uint8_t flags,int64_t imm)
{
	if (arm)
	{
		uint32_t write = 0;
		write |= flags << 28;
		write |= 1 << 27;
		write |= 1 << 25;
		write |= l << 24;
		if (imm % 4 != 0)
		{
			yyerror("branch offset has to be a multiple of 4");
			return;
		}
		bool minus = false;
		if (imm < 0)
		{
			minus = true;
			imm = - imm;
		}
		imm = imm >> 2;
		if (imm >= (2 << 22)) // bit 23 is the sign bit
		{
			yyerror("branch offset is too big");
			return;
		}
		if (minus)
		{
			imm = (-imm) & 0xffffff;
		}
		write |= imm;
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		yyerror("only arm instructions are currently supported");
		return;
	}
	yyerror("unsupported instruction");
}




void assemble_mem_half_signed_label(uint8_t l,uint8_t width,uint8_t flags, int64_t reg1,char* label,enum addressing_mode addressing,uint8_t update_reg)
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
					return;
				}
				s = 1;
				h = 1;
				break;
			case 4:
				if (l == 0)
				{
					yyerror("bh is only supported for ldr instructions, use strb instead");
					return;
				}
				s = 1;
				break;
			default:
				yyerror("invalid width");
				return;
		}
		uint32_t write = 0;
		write |= flags << 28;
		write |= 1 << 22;
		write |= l << 20;
		write |= reg1 << 12;
		write |= 15 << 16; // labels are always pc-relative
		write |= 1 << 7;
		write |= 1 << 4;
		write |= s << 6;
		write |= h << 5;
		
		if (current_section == -1)
		{
			yyerror("define a section first");
			return;
		}
		struct fixup *f = malloc(sizeof(struct fixup));
		if (f == NULL)
		{
			yyerror("Out of Memory!");
			return;
		}
		f->name = strdup(label);
		f->offset = sections[current_section]->nextindex;
		f->section = current_section;
		f->fixup_type = FIXUP_MEM_H;
		add_fixup(f);
		
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
				return;
			}
			write |= 1 << 21;
		}
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		yyerror("only arm instructions are currently supported");
		return;
	}
	yyerror("unsupported instruction");
}


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
					return;
				}
				s = 1;
				h = 1;
				break;
			case 4:
				if (l == 0)
				{
					yyerror("bh is only supported for ldr instructions, use strb instead");
					return;
				}
				s = 1;
				break;
			default:
				yyerror("invalid width");
				return;
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
			return;
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
				return;
			}
			write |= 1 << 21;
		}
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		yyerror("only arm instructions are currently supported");
		return;
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
					return;
				}
				s = 1;
				h = 1;
				break;
			case 4:
				if (l == 0)
				{
					yyerror("bh is only supported for ldr instructions, use strb instead");
					return;
				}
				s = 1;
				break;
			default:
				yyerror("invalid width");
				return;
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
				return;
			}
			write |= 1 << 21;
		}
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		yyerror("only arm instructions are currently supported");
		return;
	}
	yyerror("unsupported instruction");
}



void assemble_mem_word_ubyte_label(uint8_t l,uint8_t b, uint8_t t,uint8_t flags,int64_t reg1,char* label,enum addressing_mode addressing,uint8_t update_reg)
{
	if (arm)
	{
		uint32_t write = 0;
		write |= flags << 28;
		write |= 1 << 26;
		write |= l << 20;
		write |= b << 22;
		write |= reg1 << 12;
		write |= 15 << 16; // always use the pc as base register for labels
		
		if (current_section == -1)
		{
			yyerror("define a section first");
			return;
		}
		struct fixup *f = malloc(sizeof(struct fixup));
		if (f == NULL)
		{
			yyerror("Out of Memory!");
			return;
		}
		f->name = strdup(label);
		f->offset = sections[current_section]->nextindex;
		f->section = current_section;
		f->fixup_type = FIXUP_MEM_W_B;
		add_fixup(f);
		
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
					return;
				}
				write |= 1 << 21;
			}
		}
		else
		{
			if (addressing != post_indexed_addressing_mode)
			{
				yyerror("*t instructions only work with post-indexed access");
				return;
			}
			write |= 1 << 21;
		}
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		yyerror("only arm instructions are currently supported");
		return;
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
			return;
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
					return;
				}
				write |= 1 << 21;
			}
		}
		else
		{
			if (addressing != post_indexed_addressing_mode)
			{
				yyerror("*t instructions only work with post-indexed access");
				return;
			}
			write |= 1 << 21;
		}
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		yyerror("only arm instructions are currently supported");
		return;
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
					return;
				}
				write |= 1 << 21;
			}
		}
		else
		{
			if (addressing != post_indexed_addressing_mode)
			{
				yyerror("*t instructions only work with post-indexed access");
				return;
			}
			write |= 1 << 21;
		}
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		yyerror("only arm instructions are currently supported");
		return;
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
			return;
		}
		if (shift_type == 0b11 && shift_val == 0)
		{
			yyerror("ror #0 is not supported");
			return;
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
					return;
				}
				write |= 1 << 21;
			}
		}
		else
		{
			if (addressing != post_indexed_addressing_mode)
			{
				yyerror("*t instructions only work with post-indexed access");
				return;
			}
			write |= 1 << 21;
		}
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		yyerror("only arm instructions are currently supported");
		return;
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
		
		if (imm < 0)
		{
			yyerror("immediate value can't be 0");
			return;
		}
		uint8_t rot_imm = 0;
		int rot = is_rotated_imm(imm,&rot_imm);
		if (rot == -1)
		{
			yyerror("immediate value cannot be rotated into a 8 bit constant");
			return;
		}
		write |= rot_imm;
		write |= rot << 8;
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		yyerror("only arm instructions are currently supported");
		return;
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
			return;
		}
		if (shift_type == 0b111)
		{
			yyerror("rrx is not supported for immediate shifts");
			return;
		}
		if (shift_type == 0b11 && shift_val == 0)
		{
			yyerror("ror #0 is not supported");
			return;
		}
		write |= shift_val << 7;
		write |= shift_type << 5;
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		yyerror("only arm instructions are currently supported");
		return;
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
		return;
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
		
		if (imm < 0)
		{
			yyerror("immediate value can't be 0");
			return;
		}
		uint8_t rot_imm = 0;
		int rot = is_rotated_imm(imm,&rot_imm);
		if (rot == -1)
		{
			yyerror("immediate value cannot be rotated into a 8 bit constant");
			return;
		}
		write |= rot_imm;
		write |= rot << 8;
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		yyerror("only arm instructions are currently supported");
		return;
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
		return;
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
			return;
		}
		if (shift_type == 0b111)
		{
			yyerror("rrx is not supported for immediate shifts");
			return;
		}
		if (shift_type == 0b11 && shift_val == 0)
		{
			yyerror("ror #0 is not supported");
			return;
		}
		write |= shift_val << 7;
		write |= shift_type << 5;
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		yyerror("only arm instructions are currently supported");
		return;
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
		return;
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
		
		if (imm < 0)
		{
			yyerror("immediate value can't be 0");
			return;
		}
		uint8_t rot_imm = 0;
		int rot = is_rotated_imm(imm,&rot_imm);
		if (rot == -1)
		{
			yyerror("immediate value cannot be rotated into a 8 bit constant");
			return;
		}
		write |= rot_imm;
		write |= rot << 8;
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		yyerror("only arm instructions are currently supported");
		return;
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
			return;
		}
		if (shift_type == 0b111)
		{
			yyerror("rrx is not supported for immediate shifts");
			return;
		}
		if (shift_type == 0b11 && shift_val == 0)
		{
			yyerror("ror #0 is not supported");
			return;
		}
		write |= shift_val << 7;
		write |= shift_type << 5;
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		yyerror("only arm instructions are currently supported");
		return;
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
		return;
	}
	yyerror("unsupported instruction");
}




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






