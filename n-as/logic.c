#include "definitions.h"
#include <math.h>

int assembler_error = 0;

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



char entry_label[50];

char asm_error_msg[200];


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
	if (sect < 0 || (uint32_t) sect >= sections_size)
	{
		yyerror("define a section first");
		return;
	}
	struct section* s = sections[sect];
	if (s == NULL)
	{
		return;
	}
	if (offset < 0)
	{
		yyerror("offset has to be > 0");
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
		if ((uint32_t) (offset+i) < s->size)
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
bool section_write(int sect,const void* data,uint32_t size,int offset)
{
	if (sect < 0 || (uint32_t) sect >= sections_size)
	{
		assembler_error = -1; yyerror("define a section first");
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
			assembler_error = -1; yyerror("Out of Memory!");
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
				assembler_error = -1; yyerror("Out of Memory!");
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
				assembler_error = -1; yyerror("Out of Memory!");
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
			assembler_error = -1; yyerror("Out of Memory!");
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
			assembler_error = -1; yyerror("Out of Memory!");
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
			assembler_error = -1; yyerror("Out of Memory!");
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
			assembler_error = -1; yyerror("Out of Memory!");
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
			assembler_error = -1; yyerror("Out of Memory!");
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
			assembler_error = -1; yyerror("Out of Memory!");
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

void label_defined(const char* label,bool global)
{
	//printf("label: %s\n",label);
	struct label *found = find_label(label);
	if (found != NULL)
	{
		if (global)
		{
			found->global = true;
		}
		return;
	}
	struct label *l = malloc(sizeof(struct label));
	if (l == NULL)
	{
		assembler_error = -1; yyerror("Out of Memory!");
		return;
	}
	l->section = -1;
	l->global = global;
	l->name = strdup(label);
	if (l->name == NULL)
	{
		assembler_error = -1; yyerror("Out of Memory!");
		return;
	}
	l->offset = -1;
	l->thumb = false;
	add_label(l);
}


void label_encountered(const char* label)
{
	//printf("label: %s\n",label);
	if (current_section == -1)
	{
		assembler_error = -1; yyerror("define a section first");
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
				if (arm)
				{
					found->thumb = false;
				}
				else
				{
					found->thumb = true;
				}
				return;
			}
			assembler_error = -1; yyerror("redefinition of label");
			return;
		}
	}
	struct label *l = malloc(sizeof(struct label));
	if (l == NULL)
	{
		assembler_error = -1; yyerror("Out of Memory!");
		return;
	}
	l->section = current_section;
	l->name = strdup(label);
	l->global = false;
	if (l->name == NULL)
	{
		assembler_error = -1; yyerror("Out of Memory!");
		return;
	}
	l->offset = sections[current_section]->nextindex;
	if (arm)
	{
		l->thumb = false;
	}
	else
	{
		l->thumb = true;
	}
	add_label(l);
}

void section_encountered(const char* section)
{
	//printf("section: %s\n",section);
	if (find_section(section) == -1)
	{
		struct section *s = malloc(sizeof(struct section));
		if (s == NULL)
		{
			assembler_error = -1; yyerror("Out of Memory!");
			return;
		}
		s->name = strdup(section);
		if (s->name == NULL)
		{
			assembler_error = -1; yyerror("Out of Memory!");
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
		for (uint32_t i = 0;i<sections_size;i++)
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
	if (max < 0)
	{
		return;
	}
	if (sections != NULL)
	{
		int size = 0;
		for (uint32_t i = 0;i<sections_size;i++)
		{
			if (sections[i] != NULL)
			{
				if (sections[i]->data == NULL)
					continue;
				if (size+sections[i]->nextindex < (uint32_t) max)
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
		for (uint32_t i = 0;i<fixups_size;i++)
		{
			if (fixups[i] != NULL)
			{
				if (fixups[i]->name != NULL)
				{
					struct label *l = find_label(fixups[i]->name);
					if (l == NULL || l->section == -1)
					{
						snprintf(asm_error_msg,190,"label not found: %s",fixups[i]->name);
						return false;
					}
					int label_offset = sections[l->section]->offset + l->offset; // offset into the binary
					int fixup_offset = sections[fixups[i]->section]->offset + fixups[i]->offset;
					int32_t diff =  label_offset - fixup_offset;
					uint32_t write = 0;
					int32_t addend = 0;
					switch (fixups[i]->fixup_type)
					{
					case FIXUP_B:
						//printf("diff: %d\n",diff);
						diff -= 8;
						if (diff % 4 != 0)
						{
							yyerror("branch offset has to be a multiple of 4");
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
							yyerror("branch offset is too big");
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
							yyerror("branch offset has to be a multiple of 2");
							return false;
						}
						uint8_t h = 0;
						if ((diff & 0b10) != 0)
						{
							h |= 1;
						}
						diff = diff >> 2;
						if (diff >= (2 << 22))
						{
							yyerror("branch offset is too big");
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
							yyerror("offset is too big");
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
							yyerror("offset is too big");
							return false;
						}
						write |= (diff & 0b1111);
						write |= ((diff & 0b11110000) >> 4) << 8;
						section_write(fixups[i]->section,&write,4,fixups[i]->offset);
						break;
					case FIXUP_MEM_W_B_ADDR:
						yyerror("no pool for immediates found!");
						return false;
						break;
					case FIXUP_LABEL_ADDR_REL:
						addend = (int32_t) fixups[i]->extra;
						//printf("addend: %d, diff: %d, offset: 0x%x\n",addend,diff,fixups[i]->offset);
						write = (uint32_t) (addend + diff); // addend is the pc-bias accounted offset from the add instruction to the place in the literal pool
						section_write(fixups[i]->section,&write,4,fixups[i]->offset);
						break;
					case FIXUP_THUMB_B:
						write = 0;
						write |= 0b111 << 13;
						minus = false;
						diff -= 4;
						if (diff < 0)
						{
							minus = true;
							diff = -diff;
						}
						if (diff % 2 != 0)
						{
							assembler_error = -1; yyerror("the branch offset has to be a multiple of 2");
							return false;
						}
						diff = diff >> 1;
						if (diff >= 0b1 << 10)
						{
							assembler_error = -1; yyerror("offset too big");
							return false;
						}
						if (minus)
						{
							diff = (-diff) & 0x7ff;
						}
						write |= diff;
						
						section_write(fixups[i]->section,&write,2,fixups[i]->offset);
						break;
					case FIXUP_THUMB_B_COND:
						write = 0;
						section_read(&write,fixups[i]->section,2,fixups[i]->offset);
						write &= ~0xff; // clear the immediate bits
						minus = false;
						diff -= 4; // pc bias
						if (diff < 0)
						{
							minus = true;
							diff = -diff;
						}
						if (diff % 2 != 0)
						{
							assembler_error = -1; yyerror("the branch offset has to be a multiple of 2");
							return false;
						}
						diff = diff >> 1;
						if (diff >= 0b1 << 7)
						{
							assembler_error = -1; yyerror("offset too big");
							return false;
						}
						if (minus)
						{
							diff = (-diff) & 0xff;
						}
						write |= diff;
						
						section_write(fixups[i]->section,&write,2,fixups[i]->offset);
						break;
					case FIXUP_THUMB_BLX:
						write = 0;
						
						
						diff -= 4;
						if (diff % 2 != 0)
						{
							assembler_error = -1; yyerror("the offset of a thumb blx instruction must be deivisible by 2!");
							return false;
						}
						diff = ((fixups[i]->offset & 0b10) | (fixups[i]->offset + diff)) - fixups[i]->offset;
						if (diff < 0)
						{
							if (-diff >= 1 << 22)
							{
								assembler_error = -1; yyerror("branch offset is too big");
								return false;
							}
						}
						else
						{
							if (diff >= 1 << 22)
							{
								assembler_error = -1; yyerror("branch offset is too big");
								return false;
							}
						}
						
						write |= 0b1111 << 12; // first part
						write |= (diff & (0x7ff << 12)) >> 12;
						
						section_write(fixups[i]->section,&write,2,fixups[i]->offset);
						write = 0;
						section_read(&write,fixups[i]->section,2,fixups[i]->offset+2);
						write &= 0b11 << 11; // keep the H field of the second instruction do differentiate between bl and blx
						write |= 0b111 << 13; // second part
						write |= (diff & 0xfff) >> 1;
						
						//printf("diff: %d\n",diff);
						
						
						section_write(fixups[i]->section,&write,2,fixups[i]->offset+2);
						break;
					case FIXUP_FIXED:
						break;
					default:
						yyerror("invalid fixup type!");
						return false;
					}
				}
				else
				{
					switch(fixups[i]->fixup_type)
					{
					case FIXUP_MEM_W_B_IMM:
						yyerror("no pool for immediates found!");
						return false;
						break;
					case FIXUP_FIXED:
						break;
					default:
						yyerror("invalid fixup type!");
						return false;
					}
				}
			}
		}
	}
	return true;
}

void next_pool_found()
{
	//printf("pool!\n");
	if	(fixups != NULL)
	{
		for (uint32_t i = 0;i<fixups_size;i++)
		{
			if (fixups[i] != NULL)
			{
				if (fixups[i]->section != current_section)
				{
					continue;
				}
				const uint32_t zero = 0;
				int padd = sections[current_section]->nextindex % 4;
				section_write(current_section,&zero,padd,-1); // if not aligned, padd to word-boundary
				int32_t offset = 0;
				uint32_t write = 0;
				uint32_t imm_offset = 0;
				switch(fixups[i]->fixup_type)
				{
				case FIXUP_MEM_W_B_IMM:
					offset = sections[current_section]->nextindex - fixups[i]->offset - 8;
					section_write(current_section,&(fixups[i]->extra),4,-1);
					fixups[i]->fixup_type = FIXUP_FIXED;
					
					write = 0;
					section_read(&write,fixups[i]->section,4,fixups[i]->offset);
					write &= ~0xfff; // clear the first 12 bits for the offset
					write &= ~ (1 << 23); // clear the positive bit
					if (offset < 0)
					{
						offset = - offset;
					}
					else
					{
						write |= 1 << 23;
					}
					if (offset >=  (1 << 12))
					{
						assembler_error = -1; yyerror("literal pool too far away!");
						return;
					}
					write |= offset;
					section_write(fixups[i]->section,&write,4,fixups[i]->offset);
					break;
				case FIXUP_MEM_W_B_ADDR:
					//printf("pool place found: 0x%x, fixup offset: 0x%x\n",sections[current_section]->nextindex,fixups[i]->offset);
					offset = sections[current_section]->nextindex - fixups[i]->offset - 8;
					section_write(current_section,&zero,4,-1); // reserves 4 bytes for the address in the literal pool
					
					write = 0;
					section_read(&write,fixups[i]->section,4,fixups[i]->offset);
					write &= ~0xfff; // clear the first 12 bits for the offset
					write &= ~ (1 << 23); // clear the positive bit
					if (offset < 0)
					{
						offset = - offset;
					}
					else
					{
						write |= 1 << 23;
					}
					if (offset >=  (1 << 12))
					{
						assembler_error = -1; yyerror("literal pool too far away!");
						return;
					}
					fixups[i]->fixup_type = FIXUP_LABEL_ADDR_REL; // change the type, so apply_fixups can try to place the address in the literal pool
					fixups[i]->extra = offset-4; // pc-bias accounted distance between the following add and the place in the literal pool.
					// The pc bias is accounted for in the offset, and the -4 is because the add is 4 bytes nearer to the literal pool. Literal pools are always after a instruction that needs fixup
					
					
					
					write |= offset;
					section_write(fixups[i]->section,&write,4,fixups[i]->offset);
					
					fixups[i]->offset = sections[current_section]->nextindex-4; // used when the fixup type is changed
					break;
				case FIXUP_THUMB_MEM_ADDR:
					//printf("pool place found: 0x%x, fixup offset: 0x%x\n",sections[current_section]->nextindex,fixups[i]->offset);
					offset = sections[current_section]->nextindex - fixups[i]->offset - 4;
					if (offset < 0)
					{
						assembler_error = -1; yyerror("negative offset for FIXUP_THUMB_MEM_ADDR!");
						return;
					}
					section_write(current_section,&zero,4,-1); // reserves 4 bytes for the address in the literal pool
					
					write = 0;
					section_read(&write,fixups[i]->section,2,fixups[i]->offset);
					write &= ~0xff; // clear the first byte, so the offset is 0
					imm_offset = offset & (~0b11); // for this thumb instruction, the first 2 bit of the pc are regarded as 0, to make everything word-aligned
					if ( (offset & 0b11) != 0)
					{
						imm_offset += 4;
					}
					imm_offset = imm_offset / 4;
					
					if (imm_offset >=  (1 << 8))
					{
						assembler_error = -1; yyerror("literal pool too far away!");
						return;
					}
					fixups[i]->fixup_type = FIXUP_LABEL_ADDR_REL; // change the type, so apply_fixups can try to place the address in the literal pool
					fixups[i]->extra = offset-2; // pc-bias accounted distance between the following add and the place in the literal pool.
					// The pc bias is accounted for in the offset, and the -2 is because the add is 2 bytes nearer to the literal pool. Literal pools are always after a instruction that needs fixup
					
					write |= imm_offset;
					
					section_write(fixups[i]->section,&write,2,fixups[i]->offset);
					
					fixups[i]->offset = sections[current_section]->nextindex-4; // used when the fixup type is changed
					break;
				case FIXUP_THUMB_MEM_IMM:
					//printf("pool place found: 0x%x, fixup offset: 0x%x\n",sections[current_section]->nextindex,fixups[i]->offset);
					offset = sections[current_section]->nextindex - fixups[i]->offset - 4;
					if (offset < 0)
					{
						assembler_error = -1; yyerror("negative offset for FIXUP_THUMB_MEM_IMM!");
						return;
					}
					fixups[i]->fixup_type = FIXUP_FIXED;
					section_write(current_section,&(fixups[i]->extra),4,-1);
					
					write = 0;
					section_read(&write,fixups[i]->section,2,fixups[i]->offset);
					write &= ~0xff; // clear the first byte, so the offset is 0
					imm_offset = offset & (~0b11); // for this thumb instruction, the first 2 bit of the pc are regarded as 0, to make everything word-aligned
					if ( (offset & 0b11) != 0)
					{
						imm_offset += 4;
					}
					imm_offset = imm_offset / 4;
					
					if (imm_offset >=  (1 << 8))
					{
						assembler_error = -1; yyerror("literal pool too far away!");
						return;
					}
					write |= imm_offset;
					
					section_write(fixups[i]->section,&write,2,fixups[i]->offset);
					break;
				}
			}
		}
	}
}




int64_t string_to_immediate(char* str, int base)
{
	errno = 0;
	char* successful = NULL;
	long long l = strtoll(str,&successful,base);
	if (errno != 0)
	{
		assembler_error = -1; yyerror("could not convert to a 32 bit integer");
		return 0xffffffffff; // bigger than any 32 bit number, so this works
	}
	if (l < 0)
	{
		if (l < -pow(2,32)) // will be used as uint32_t anyways
		{
			assembler_error = -1; yyerror("could not convert to a 32 bit integer");
			return 0xffffffffff;
		}
	}
	else
	{
		if (l > pow(2,32)) // will be used as uint32_t anyways
		{
			assembler_error = -1; yyerror("could not convert to a 32 bit integer");
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
		
		
		assembler_error = -1; yyerror("unsupported instruction");
	}
	else
	{
		assembler_error = -1; yyerror("only arm instructions are currently supported");
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









void assemble_entry(const char* entry)
{
	snprintf(entry_label,45,"%s",entry);
}



void assemble_align(int64_t align)
{
	if (current_section == -1)
	{
		assembler_error = -1; yyerror("define a section first");
		return;
	}
	if (align < 0)
	{
		assembler_error = -1; yyerror("alignment has to be positive!");
		return;
	}
	if (align == 0)
	{
		return;
	}
	assemble_zero(align - (sections[current_section]->nextindex % align));
}

void assemble_ascii_zero(const char* string)
{
	assemble_ascii(string);
	uint8_t zero = 0;
	section_write(current_section,&zero,1,-1);
}
void assemble_ascii(const char* string)
{
	if (current_section == -1)
	{
		assembler_error = -1; yyerror("define a section first");
		return;
	}
	section_write(current_section,string,strlen(string),-1);
}

void assemble_fill(int64_t repeat, int64_t size, int64_t value)
{
	if (current_section == -1)
	{
		assembler_error = -1; yyerror("define a section first");
		return;
	}
	if (size < 1 || size > 4)
	{
		assembler_error = -1; yyerror("size has to be >= 1 and <= 4");
		return;
	}
	if (repeat < 1)
	{
		assembler_error = -1; yyerror("repeat has to be >= 1");
		return;
	}
	for (int64_t i = 0;i<repeat;i++)
	{
		section_write(current_section,&value,size,-1);
	}
}
void assemble_zero(int64_t bytes)
{
	if (current_section == -1)
	{
		assembler_error = -1; yyerror("define a section first");
		return;
	}
	if (bytes < 0)
	{
		assembler_error = -1; yyerror("number of bytes has to be positive!");
		return;
	}
	if (bytes == 0)
	{
		return;
	}
	uint8_t zero = 0;
	for (int64_t i = 0;i<bytes;i++)
	{
		section_write(current_section,&zero,1,-1);
	}
}
void assemble_space(int64_t bytes, int64_t value)
{
	if (current_section == -1)
	{
		assembler_error = -1; yyerror("define a section first");
		return;
	}
	if (value > 0xff)
	{
		assembler_error = -1; yyerror("value must fit into one byte");
		return;
	}
	for (int64_t i = 0;i<bytes;i++)
	{
		section_write(current_section,&value,1,-1);
	}
}




void assemble_undef()
{
	if (arm)
	{
		uint32_t write = 0;
		write |= ALWAYS << 28;
		write |= 0b1111111 << 20;
		write |= 0b1111 << 4;
		
		section_write(current_section,&write,4,-1);
	}
	else
	{
		uint16_t write = 0;
		write |= 0b1101 << 12;;
		write |= 0b111 << 9;
		
		section_write(current_section,&write,2,-1);
	}
}
void assemble_bkpt()
{
	if (arm)
	{
		uint32_t write = 0;
		write |= 0b1110 << 28;
		write |= 1 << 24;
		write |= 1 << 21;
		write |= 0b111 << 4;
		
		section_write(current_section,&write,4,-1);
	}
	else
	{
		uint16_t write = 0;
		write |= 0b1011111 << 9;
		
		section_write(current_section,&write,2,-1);
	}
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
		assembler_error = -1; yyerror("msr and mrs are not allowed");
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
			assembler_error = -1; yyerror("immediate value can't be 0");
			return;
		}
		uint8_t rot_imm = 0;
		int rot = is_rotated_imm(imm,&rot_imm);
		if (rot == -1)
		{
			assembler_error = -1; yyerror("immediate value cannot be rotated into a 8 bit constant");
			return;
		}
		write |= rot_imm;
		write |= rot << 8;
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		assembler_error = -1; yyerror("only arm instructions are currently supported");
		return;
	}
	assembler_error = -1; yyerror("unsupported instruction");
}

void assemble_msr_reg(uint8_t flags,uint8_t spsr,uint8_t psr_fields, int64_t reg)
{
	if ((assembler_flags & ASSEMBLER_PSR_ALLOWED) == 0)
	{
		assembler_error = -1; yyerror("msr and mrs are not allowed");
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
		assembler_error = -1; yyerror("only arm instructions are currently supported");
		return;
	}
	assembler_error = -1; yyerror("unsupported instruction");
}

void assemble_mrs(uint8_t flags, int64_t reg,uint8_t spsr)
{
	if ((assembler_flags & ASSEMBLER_PSR_ALLOWED) == 0)
	{
		assembler_error = -1; yyerror("msr and mrs are not allowed");
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
		assembler_error = -1; yyerror("only arm instructions are currently supported");
		return;
	}
	assembler_error = -1; yyerror("unsupported instruction");
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
		uint16_t write = 0;
		if (flags != ALWAYS)
		{
			assembler_error = -1; yyerror("thumb blx instructions are unconditional");
			return;
		}
		write |= 0b1 << 14;
		write |= 0b1111 << 7;
		write |= reg << 3;
		
		section_write(current_section,&write,2,-1);
		return;
	}
	assembler_error = -1; yyerror("unsupported instruction");
}

void assemble_blx_label(const char* label)
{
	if (arm)
	{
		uint32_t write = 0;
		write |= 0b1111101 << 25;;
		
		
		if (current_section == -1)
		{
			assembler_error = -1; yyerror("define a section first");
			return;
		}
		struct fixup *f = malloc(sizeof(struct fixup));
		if (f == NULL)
		{
			assembler_error = -1; yyerror("Out of Memory!");
			return;
		}
		label_defined(label,false); // weakly define labels when encountered as arguments of instructions
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
		if (current_section == -1)
		{
			assembler_error = -1; yyerror("define a section first");
			return;
		}
		struct fixup *f = malloc(sizeof(struct fixup));
		if (f == NULL)
		{
			assembler_error = -1; yyerror("Out of Memory!");
			return;
		}
		label_defined(label,false); // weakly define labels when encountered as arguments of instructions
		f->name = strdup(label);
		f->offset = sections[current_section]->nextindex;
		f->section = current_section;
		f->fixup_type = FIXUP_THUMB_BLX;
		add_fixup(f);
		
		assemble_blx_imm(0);
		return;
	}
	assembler_error = -1; yyerror("unsupported instruction");
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
			assembler_error = -1; yyerror("branch offset has to be a multiple of 2");
			return;
		}
		if ((imm & 0b10) != 0)
		{
			write |= 1 << 24;
		}
		imm = imm >> 2;
		if (imm >= (2 << 22))
		{
			assembler_error = -1; yyerror("branch offset is too big");
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
		uint16_t write = 0;
		imm -= 4;
		if (imm % 2 != 0)
		{
			assembler_error = -1; yyerror("the offset of a thumb blx instruction must be divisible by 2!");
			return;
		}
		if (current_section == -1)
		{
			assembler_error = -1; yyerror("define a section first!");
			return;
		}
		imm = ((sections[current_section]->nextindex & 0b10) | (sections[current_section]->nextindex + imm)) - sections[current_section]->nextindex;
		if (imm < 0)
		{
			if (-imm >= 1 << 22)
			{
				assembler_error = -1; yyerror("branch offset is too big");
				return;
			}
		}
		else
		{
			if (imm >= 1 << 22)
			{
				assembler_error = -1; yyerror("branch offset is too big");
				return;
			}
		}
		
		
		write |= 0b1111 << 12; // first part
		write |= (imm & (0x7ff << 12)) >> 12;
		
		
		
		section_write(current_section,&write,2,-1);
		write = 0;
		write |= 0b11101 << 11; // second part
		write |= (imm & 0xfff) >> 1;
		
		
		
		section_write(current_section,&write,2,-1);
		return;
	}
	assembler_error = -1; yyerror("unsupported instruction");
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
		uint16_t write = 0;
		if (flags != ALWAYS)
		{
			assembler_error = -1; yyerror("thumb blx instructions are unconditional");
			return;
		}
		write |= 0b1 << 14;
		write |= 0b111 << 8;
		write |= reg << 3;
		
		section_write(current_section,&write,2,-1);
		return;
	}
	assembler_error = -1; yyerror("unsupported instruction");
}
























void assemble_mul(uint8_t inst,uint8_t mul_width,uint8_t update_flags, uint8_t flags,int64_t reg1,int64_t reg2,int64_t reg3,int64_t reg4)
{
	if (arm)
	{
		uint32_t write = 0;
		write |= flags << 28;
		if (reg3 == -1)
		{
			assembler_error = -1; yyerror("arm multiply needs at least 3 registers");
			return;
		}
		if (inst < 4)
		{
			if (mul_width != 0)
			{
				assembler_error = -1; yyerror("this multiplication instruction doesn't need a width specifier");
				return;
			}
			write |= 0b1001 << 4;
			write |= update_flags << 20;
			if (inst > 1)
			{
				if (reg4 == -1)
				{
					assembler_error = -1; yyerror("not enough registers");
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
						assembler_error = -1; yyerror("not enough registers");
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
						assembler_error = -1; yyerror("instruction needs b/t");
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
						assembler_error = -1; yyerror("s suffix not allowed for this instruction");
						return;
					}
					switch (inst)
					{
						case 4:
							write |= 0b101 << 22;
							if (mul_width < 3)
							{
								assembler_error = -1; yyerror("invalid multiply suffix");
								return;
							}
							break;
						case 5:
							write |= 1 << 24;
							if (mul_width < 3)
							{
								assembler_error = -1; yyerror("invalid multiply suffix");
								return;
							}
							break;
						case 7:
							write |= 0b1011 << 21;
							if (mul_width < 3)
							{
								assembler_error = -1; yyerror("invalid multiply suffix");
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
								assembler_error = -1; yyerror("this instruction needs 4 operands");
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
								assembler_error = -1; yyerror("this instruction needs 4 operands");
								return;
							}
							break;
						case 7:
							write |= reg1 << 16;
							write |= reg2;
							write |= reg3 << 8;
							if (reg4 != -1)
							{
								assembler_error = -1; yyerror("this instruction only needs 3 operands");
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
					assembler_error = -1; yyerror("invalid multiply suffix");
					return;
				}
				if (mul_width == 2)
				{
					write |= 1 << 6;
				}
				if (update_flags != 0)
				{
					assembler_error = -1; yyerror("s suffix not allowed for this instruction");
					return;
				}
				write |= 1 << 7;
				write |= 0b1001 << 21;
				if (inst == 6 && reg4 != -1)
				{
					assembler_error = -1; yyerror("this instruction only needs 3 operands");
					return;
				}
				if (inst == 8 && reg4 == -1)
				{
					assembler_error = -1; yyerror("this instruction needs 4 operands");
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
			assembler_error = -1; yyerror("unsupported instruction");
			return;
		}
		
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		if (flags != ALWAYS)
		{
			assembler_error = -1; yyerror("thumb data-processing instructions are unconditional");
			return;
		}
		
		if (update_flags)
		{
			assembler_error = -1; yyerror("thumb data-processing instructions always update the flags, except when using high registers");
			return;
		}
		
		if (mul_width != 0)
		{
			assembler_error = -1; yyerror("thumb mul doesn't need width specifiers");
			return;
		}
		if (reg3 != -1 || reg4 != -1)
		{
			assembler_error = -1; yyerror("thumb mul only needs 2 registers");
			return;
		}
		if (inst != 0)
		{
			assembler_error = -1; yyerror("thumb only supports mul");
			return;
		}
		
		if (reg1 >= 0b1000 || reg2 >= 0b1000)
		{
			assembler_error = -1; yyerror("thumb data-processing instructions can only use low registers, with some exceptions");
			return;
		}
		
		uint16_t write = 0;
		write |= 1 << 14;
		write |= 0b1101 << 6;
		write |= reg1;
		write |= reg2 << 3;
		
		section_write(current_section,&write,2,-1);
		return;
	}
	assembler_error = -1; yyerror("unsupported instruction");
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
		assembler_error = -1; yyerror("only arm instructions are currently supported");
		return;
	}
	assembler_error = -1; yyerror("unsupported instruction");
}


void assemble_coproc(uint8_t mrc,uint8_t flags,int64_t coproc,int64_t opcode1,int64_t reg,int64_t coproc_reg1,int64_t coproc_reg2,int64_t opcode2)
{
	if ((assembler_flags & ASSEMBLER_COPROCESSOR_ALLOWED) == 0)
	{
		assembler_error = -1; yyerror("coprocessor instructions are disabled");
		return;
	}
	if (arm)
	{
		uint32_t write = 0;
		if (coproc > 15)
		{
			assembler_error = -1; yyerror("invalid coprocessor number");
			return;
		}
		if (opcode1 >= (1 << 3))
		{
			assembler_error = -1; yyerror("invalid opcode 1");
			return;
		}
		if (opcode2 >= (1 << 3))
		{
			assembler_error = -1; yyerror("invalid opcode 2");
			return;
		}
		if (coproc_reg1 >= (1 << 4))
		{
			assembler_error = -1; yyerror("invalid coprocessor register");
			return;
		}
		if (coproc_reg2 >= (1 << 4))
		{
			assembler_error = -1; yyerror("invalid coprocessor register");
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
		assembler_error = -1; yyerror("only arm instructions are currently supported");
		return;
	}
	assembler_error = -1; yyerror("unsupported instruction");
}





void assemble_swi(uint8_t flags, int64_t imm)
{
	if ((assembler_flags & ASSEMBLER_SWI_ALLOWED) == 0)
	{
		assembler_error = -1; yyerror("swi instructions are disabled");
		return;
	}
	if (arm)
	{
		uint32_t write = 0;
		write |= flags << 28;
		write |= 0b1111 << 24;
		if (imm < 0)
		{
			assembler_error = -1; yyerror("swi numbers are always positive");
			return;
		}
		if (imm >= (1 << 24))
		{
			assembler_error = -1; yyerror("swi number too big!");
			return;
		}
		write |= imm;
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		uint16_t write = 0;
		if (flags != ALWAYS)
		{
			assembler_error = -1; yyerror("thumb instructions are unconditional");
			return;
		}
		if (imm < 0)
		{
			assembler_error = -1; yyerror("swi numbers are always positive");
			return;
		}
		if (imm >= 1 << 8)
		{
			assembler_error = -1; yyerror("swi number too big!");
			return;
		}
		write |= 0b11011111 << 8;
		write |= imm;
		
		section_write(current_section,&write,2,-1);
		return;
	}
	assembler_error = -1; yyerror("unsupported instruction");
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
		assembler_error = -1; yyerror("only arm instructions are currently supported");
		return;
	}
	assembler_error = -1; yyerror("unsupported instruction");
}




void assemble_mem_multiple(uint8_t l,uint8_t adr_mode,uint8_t flags, int64_t reg1,uint8_t update_reg,uint16_t reglist,uint8_t user_mode_regs)
{
	if (reglist == 0)
	{
		assembler_error = -1; yyerror("register list can't be empty");
		return;
	}
	if (arm)
	{
		uint32_t write = 0;
		if (update_reg == 1 && l == false && ((reglist >> reg1) & 0b1) == 1 && __builtin_ctz(reglist) != reg1)
		{
			assembler_error = -1; yyerror("if a base register is included in the register list, it has to be the first one");
			return;
		}
		
		
		write |= flags << 28;
		write |= 1 << 27;
		write |= user_mode_regs << 22;
		write |= update_reg << 21;
		write |= l << 20;
		write |= reg1 << 16;
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
		uint16_t write = 0;
		if (flags != ALWAYS)
		{
			assembler_error = -1; yyerror("thumb instructions are unconditional");
			return;
		}
		if (! update_reg)
		{
			assembler_error = -1; yyerror("the base register has to be updated in thumb ldm/stm instructions");
			return;
		}
		if (user_mode_regs)
		{
			assembler_error = -1; yyerror("user-mode registers can't be used in thumb ldm/stm instructions");
			return;
		}
		if (reg1 >= 0b1000)
		{
			if (reg1 == 13) // push/pop
			{
				if (l && adr_mode == 0)
				{
					write |= 0b101111 << 10;
					if (reglist != (reglist & 0x80ff)) // checks if any high registers except pc are used
					{
						assembler_error = -1; yyerror("thumb push/pop instructions can't use the high registers, except the pc");
						return;
					}
					if ( ((reglist >> 15) & 0b1) == 1)
					{
						reglist = reglist & 0xff;
						write |= 1 << 8;
					}
					write |= reglist;
					
					section_write(current_section,&write,2,-1);
					return;
				}
				if ( (! l) && adr_mode == 3)
				{
					write |= 0b101101 << 10;
					if (reglist != (reglist & 0x40ff)) // checks if any high registers except lr are used
					{
						assembler_error = -1; yyerror("thumb push/pop instructions can't use the high registers, except the pc");
						return;
					}
					if ( ((reglist >> 14) & 0b1) == 1)
					{
						reglist = reglist & 0xff;
						write |= 1 << 8;
					}
					write |= reglist;
					
					section_write(current_section,&write,2,-1);
					return;
					
					section_write(current_section,&write,2,-1);
					return;
				}
			}
			assembler_error = -1; yyerror("thumb instructions can only use low registers, with some exceptions");
			return;
		}
		if (adr_mode != 0)
		{
			assembler_error = -1; yyerror("thumb only supports ldmia/stmia");
			return;
		}
		if (reglist != (reglist & 0xff)) // checks if any high registers are used
		{
			assembler_error = -1; yyerror("thumb ldm/stm instructions can't use the high registers");
			return;
		}
		
		
		write |= 0b11 << 14;
		if (l)
		{
			write |= 1 << 11;
		}
		write |= reg1 << 8;
		write |= reglist;
		
		
		section_write(current_section,&write,2,-1);
		return;
	}
	assembler_error = -1; yyerror("unsupported instruction");
}




void assemble_branch_label(uint8_t l,uint8_t flags,const char* label)
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
			assembler_error = -1; yyerror("define a section first");
			return;
		}
		struct fixup *f = malloc(sizeof(struct fixup));
		if (f == NULL)
		{
			assembler_error = -1; yyerror("Out of Memory!");
			return;
		}
		label_defined(label,false); // weakly define labels when encountered as arguments of instructions
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
		if (current_section == -1)
		{
			assembler_error = -1; yyerror("define a section first");
			return;
		}
		struct fixup *f = malloc(sizeof(struct fixup));
		if (f == NULL)
		{
			assembler_error = -1; yyerror("Out of Memory!");
			return;
		}
		label_defined(label,false); // weakly define labels when encountered as arguments of instructions
		f->name = strdup(label);
		f->offset = sections[current_section]->nextindex;
		f->section = current_section;
		if (flags != ALWAYS)
		{
			f->fixup_type = FIXUP_THUMB_B_COND;
		}
		else
		{
			f->fixup_type = FIXUP_THUMB_B;
		}
		if (l == 1)
		{
			f->fixup_type = FIXUP_THUMB_BLX;
		}
		assemble_branch(l,flags,0);
		
		if (assembler_error != -1)
		{
			add_fixup(f);
		}
		return;
	}
	assembler_error = -1; yyerror("unsupported instruction");
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
			assembler_error = -1; yyerror("branch offset has to be a multiple of 4");
			return;
		}
		bool minus = false;
		if (imm < 0)
		{
			minus = true;
			imm = - imm;
		}
		imm = imm >> 2;
		if (imm >= (1 << 23)) // bit 23 is the sign bit
		{
			assembler_error = -1; yyerror("branch offset is too big");
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
		uint16_t write = 0;
		if (flags != ALWAYS)
		{
			if (l)
			{
				assembler_error = -1; yyerror("thumb bl/blx is unconditional");
				return;
			}
			write |= 0b1101 << 12;
			write |= flags << 8;
			bool minus = false;
			imm -= 4; // pc bias
			if (imm < 0)
			{
				minus = true;
				imm = -imm;
			}
			if (imm % 2 != 0)
			{
				assembler_error = -1; yyerror("the branch offset has to be a multiple of 2");
				return;
			}
			imm = imm >> 1;
			if (imm >= 0b1 << 7)
			{
				assembler_error = -1; yyerror("offset too big");
				return;
			}
			if (minus)
			{
				imm = (-imm) & 0xff;
			}
			write |= imm;
		}
		else
		{
			if (l)
			{
				imm -= 4;
				
				if (imm < 0)
				{
					if (-imm >= 1 << 22)
					{
						assembler_error = -1; yyerror("branch offset is too big");
						return;
					}
				}
				else
				{
					if (imm >= 1 << 22)
					{
						assembler_error = -1; yyerror("branch offset is too big");
						return;
					}
				}
				
				
				write |= 0b1111 << 12; // first part
				write |= (imm & (0x7ff << 12)) >> 12;
				
				
				
				section_write(current_section,&write,2,-1);
				write = 0;
				write |= 0b11111 << 11; // second part
				write |= (imm & 0xfff) >> 1;
				
				
				
				section_write(current_section,&write,2,-1);
				return;
			}
			else
			{
				write |= 0b111 << 13;
				bool minus = false;
				imm -= 4; // pc bias
				if (imm < 0)
				{
					minus = true;
					imm = -imm;
				}
				if (imm % 2 != 0)
				{
					assembler_error = -1; yyerror("the branch offset has to be a multiple of 2");
					return;
				}
				//printf("imm: %d\n",imm);
				imm = imm >> 1;
				if (imm >= 0b1 << 10)
				{
					assembler_error = -1; yyerror("offset too big");
					return;
				}
				if (minus)
				{
					imm = (-imm) & 0x7ff;
				}
				
				write |= imm;
			}
		}
		section_write(current_section,&write,2,-1);
		return;
	}
	assembler_error = -1; yyerror("unsupported instruction");
}




void assemble_mem_half_signed_label(uint8_t l,uint8_t width,uint8_t flags, int64_t reg1,const char* label,enum addressing_mode addressing,uint8_t update_reg)
{
	if (arm)
	{
		if (current_section == -1)
		{
			assembler_error = -1; yyerror("define a section first");
			return;
		}
		struct fixup *f = malloc(sizeof(struct fixup));
		if (f == NULL)
		{
			assembler_error = -1; yyerror("Out of Memory!");
			return;
		}
		label_defined(label,false); // weakly define labels when encountered as arguments of instructions
		f->name = strdup(label);
		f->offset = sections[current_section]->nextindex;
		f->section = current_section;
		f->fixup_type = FIXUP_MEM_H;
		assemble_mem_half_signed_imm(l,width,flags,reg1,15,0,offset_addressing_mode,update_reg);
		if (assembler_error != -1)
		{
			add_fixup(f);
		}
		else
		{
			free(f);
		}
		return;
	}
	else
	{
		assembler_error = -1; yyerror("only arm instructions are currently supported");
		return;
	}
	assembler_error = -1; yyerror("unsupported instruction");
}


void assemble_mem_half_signed_imm(uint8_t l,uint8_t width,uint8_t flags, int64_t reg1, int64_t reg2, int64_t imm,enum addressing_mode addressing,uint8_t update_reg)
{
	if (arm)
	{
		uint8_t s = 0, h = 0;
		switch (width)
		{
			case 1:
				if (reg1 % 2 != 0)
				{
					assembler_error = -1; yyerror("ldrd/strd has to use a even register");
					return;
				}
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
					assembler_error = -1; yyerror("sh is only supported for ldr instructions, use strh instead");
					return;
				}
				s = 1;
				h = 1;
				break;
			case 4:
				if (l == 0)
				{
					assembler_error = -1; yyerror("bh is only supported for ldr instructions, use strb instead");
					return;
				}
				s = 1;
				break;
			default:
				assembler_error = -1; yyerror("invalid width");
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
			assembler_error = -1; yyerror("offset is too big");
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
				assembler_error = -1; yyerror("the base register is always updated in post-indexed addressing");
				return;
			}
			write |= 1 << 21;
		}
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		uint16_t write = 0;
		if (flags != ALWAYS)
		{
			assembler_error = -1; yyerror("thumb instructions are unconditional");
			return;
		}
		if (update_reg)
		{
			assembler_error = -1; yyerror("the base register can't be updated in thumb");
			return;
		}
		if (addressing == post_indexed_addressing_mode)
		{
			assembler_error = -1; yyerror("thumb doesn't support post-indexed addressing");
			return;
		}
		if (imm < 0)
		{
			assembler_error = -1; yyerror("the offset has to be positive");
			return;
		}
		if (reg1 >= 0b1000 || reg2 >= 0b1000)
		{
			assembler_error = -1; yyerror("thumb instructions can only use low registers, with some exceptions");
			return;
		}
		
		if (width == 1)
		{
			assembler_error = -1; yyerror("thumb has no ldrd/strd instructions");
			return;
		}
		
		write |= 1 << 15;
		
		if (! l)
		{
			if (width == 3 || width == 4)
			{
				assembler_error = -1; yyerror("there are no strsh/strsb instructions. Use strh and strb instead");
				return;
			}
		}
		else
		{
			if (width == 3 || width == 4)
			{
				assembler_error = -1; yyerror("ldrsh/ldrsb can not be used with immediate offsets in thumb");
				return;
			}
			write |= 1 << 11;
		}
		
		if (imm % 2 != 0)
		{
			assembler_error = -1; yyerror("offset has to be a multiple of 2");
			return;
		}
		imm = imm / 2;
		
		if (imm >= 0b1 << 5)
		{
			assembler_error = -1; yyerror("offset too large");
			return;
		}
		write |= imm << 6;
		write |= reg2 << 3;
		write |= reg1;
		
		section_write(current_section,&write,2,-1);
		return;
	}
	assembler_error = -1; yyerror("unsupported instruction");
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
					assembler_error = -1; yyerror("sh is only supported for ldr instructions, use strh instead");
					return;
				}
				s = 1;
				h = 1;
				break;
			case 4:
				if (l == 0)
				{
					assembler_error = -1; yyerror("bh is only supported for ldr instructions, use strb instead");
					return;
				}
				s = 1;
				break;
			default:
				assembler_error = -1; yyerror("invalid width");
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
				assembler_error = -1; yyerror("the base register is always updated in post-indexed addressing");
				return;
			}
			write |= 1 << 21;
		}
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		uint16_t write = 0;
		if (flags != ALWAYS)
		{
			assembler_error = -1; yyerror("thumb instructions are unconditional");
			return;
		}
		if (update_reg)
		{
			assembler_error = -1; yyerror("the base register can't be updated in thumb");
			return;
		}
		if (addressing == post_indexed_addressing_mode)
		{
			assembler_error = -1; yyerror("thumb doesn't support post-indexed addressing");
			return;
		}
		if (u == 0)
		{
			assembler_error = -1; yyerror("thumb memory instructions can only add a register offset, use neg rx before");
			return;
		}
		if (reg1 >= 0b1000 || reg2 >= 0b1000 || reg3 >= 0b1000)
		{
			assembler_error = -1; yyerror("thumb instructions can only use low registers, with some exceptions");
			return;
		}
		
		if (width == 1)
		{
			assembler_error = -1; yyerror("thumb has no ldrd/strd instructions");
			return;
		}
		
		write |= 0b101 << 12;
		write |= 1 << 9;
		
		if (! l)
		{
			if (width == 3 || width == 4)
			{
				assembler_error = -1; yyerror("there are no strsh/strsb instructions. Use strh and strb instead");
				return;
			}
		}
		else
		{
			if (width == 2) // h
			{
				write |= 1 << 11;
			}
			if (width == 3) // sh
			{
				write |= 1 << 11;
				write |= 1 << 10;
			}
			if (width == 4) // sb
			{
				write |= 1 << 10;
			}
		}
		
		write |= reg3 << 6;
		write |= reg2 << 3;
		write |= reg1;
		
		
		section_write(current_section,&write,2,-1);
		return;
	}
	assembler_error = -1; yyerror("unsupported instruction");
}

void assemble_mem_word_ubyte_imm_big(uint8_t l,uint8_t b, uint8_t t,uint8_t flags,int64_t reg1,uint32_t imm,enum addressing_mode addressing,uint8_t update_reg)
{
	if (l != 1)
	{
		assembler_error = -1; yyerror("you can only load an immediate value");
		return;
	}
	if (arm)
	{
		if (current_section == -1)
		{
			assembler_error = -1; yyerror("define a section first");
			return;
		}
		struct fixup *f = malloc(sizeof(struct fixup));
		if (f == NULL)
		{
			assembler_error = -1; yyerror("Out of Memory!");
			return;
		}
		f->name = NULL;
		f->offset = sections[current_section]->nextindex;
		f->section = current_section;
		f->fixup_type = FIXUP_MEM_W_B_IMM;
		f->extra = imm;
		assemble_mem_word_ubyte_imm(l,b,t,flags,reg1,15,0,offset_addressing_mode,update_reg);
		if (assembler_error != -1)
		{
			add_fixup(f);
		}
		else
		{
			free(f);
		}
		return;
	}
	else
	{
		if (current_section == -1)
		{
			assembler_error = -1; yyerror("define a section first");
			return;
		}
		struct fixup *f = malloc(sizeof(struct fixup));
		if (f == NULL)
		{
			assembler_error = -1; yyerror("Out of Memory!");
			return;
		}
		f->name = NULL;
		f->offset = sections[current_section]->nextindex;
		f->section = current_section;
		f->fixup_type = FIXUP_THUMB_MEM_IMM;
		f->extra = imm;
		assemble_mem_word_ubyte_imm(l,b,t,flags,reg1,15,0,offset_addressing_mode,update_reg);
		if (assembler_error != -1)
		{
			add_fixup(f);
		}
		else
		{
			free(f);
		}
		return;
	}
	assembler_error = -1; yyerror("unsupported instruction");
}

void assemble_mem_word_ubyte_label_address(uint8_t l,uint8_t b, uint8_t t,uint8_t flags,int64_t reg1,const char* label,enum addressing_mode addressing,uint8_t update_reg)
{
	if (l != 1)
	{
		assembler_error = -1; yyerror("you can only load the address of a label");
		return;
	}
	if (arm)
	{
		if (current_section == -1)
		{
			assembler_error = -1; yyerror("define a section first");
			return;
		}
		struct fixup *f = malloc(sizeof(struct fixup));
		if (f == NULL)
		{
			assembler_error = -1; yyerror("Out of Memory!");
			return;
		}
		label_defined(label,false); // weakly define labels when encountered as arguments of instructions
		f->name = strdup(label);
		f->offset = sections[current_section]->nextindex;
		f->section = current_section;
		f->fixup_type = FIXUP_MEM_W_B_ADDR;
		assemble_mem_word_ubyte_imm(l,b,t,flags,reg1,15,0,offset_addressing_mode,update_reg);
		assemble_data_proc_reg_reg_reg(0b0100,ALWAYS,0,reg1,reg1,15); // add reg1, reg1, pc
		if (assembler_error != -1)
		{
			add_fixup(f);
		}
		else
		{
			free(f);
		}
		return;
	}
	else
	{
		if (current_section == -1)
		{
			assembler_error = -1; yyerror("define a section first");
			return;
		}
		struct fixup *f = malloc(sizeof(struct fixup));
		if (f == NULL)
		{
			assembler_error = -1; yyerror("Out of Memory!");
			return;
		}
		label_defined(label,false); // weakly define labels when encountered as arguments of instructions
		f->name = strdup(label);
		f->offset = sections[current_section]->nextindex;
		f->section = current_section;
		f->fixup_type = FIXUP_THUMB_MEM_ADDR;
		assemble_mem_word_ubyte_imm(l,b,t,flags,reg1,15,0,offset_addressing_mode,update_reg);
		assemble_data_proc_reg_reg(0b0100,ALWAYS,0,reg1,15); // add reg1, pc
		if (assembler_error != -1)
		{
			add_fixup(f);
		}
		else
		{
			free(f);
		}
		return;
	}
	assembler_error = -1; yyerror("unsupported instruction");
}

void assemble_mem_word_ubyte_label(uint8_t l,uint8_t b, uint8_t t,uint8_t flags,int64_t reg1,const char* label,enum addressing_mode addressing,uint8_t update_reg)
{
	if (arm)
	{
		if (current_section == -1)
		{
			assembler_error = -1; yyerror("define a section first");
			return;
		}
		struct fixup *f = malloc(sizeof(struct fixup));
		if (f == NULL)
		{
			assembler_error = -1; yyerror("Out of Memory!");
			return;
		}
		label_defined(label,false); // weakly define labels when encountered as arguments of instructions
		f->name = strdup(label);
		f->offset = sections[current_section]->nextindex;
		f->section = current_section;
		f->fixup_type = FIXUP_MEM_W_B;
		assemble_mem_word_ubyte_imm(l,b,t,flags,reg1,15,0,offset_addressing_mode,update_reg);
		if (assembler_error != -1)
		{
			add_fixup(f);
		}
		else
		{
			free(f);
		}
		return;
	}
	else
	{
		assembler_error = -1; yyerror("loading or storing directly to a label is disabled for thumb, because you would have to align the label to 4 bytes inside the code, storing isn't possible and access only to data after the load");
		return;
	}
	assembler_error = -1; yyerror("unsupported instruction");
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
			assembler_error = -1; yyerror("offset is too big");
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
					assembler_error = -1; yyerror("the base register is always updated in post-indexed addressing");
					return;
				}
				write |= 1 << 21;
			}
		}
		else
		{
			if (addressing != post_indexed_addressing_mode)
			{
				assembler_error = -1; yyerror("*t instructions only work with post-indexed access");
				return;
			}
			write |= 1 << 21;
		}
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		uint16_t write = 0;
		if (flags != ALWAYS)
		{
			assembler_error = -1; yyerror("thumb instructions are unconditional");
			return;
		}
		if (update_reg)
		{
			assembler_error = -1; yyerror("the base register can't be updated in thumb");
			return;
		}
		if (addressing == post_indexed_addressing_mode)
		{
			assembler_error = -1; yyerror("thumb doesn't support post-indexed addressing");
			return;
		}
		if (imm < 0)
		{
			assembler_error = -1; yyerror("the offset has to be positive");
			return;
		}
		if (reg1 >= 0b1000 || reg2 >= 0b1000)
		{
			if (reg1 >= 0b1000)
			{
				assembler_error = -1; yyerror("thumb instructions can only use low registers, with some exceptions");
				return;
			}
			else
			{
				if (! b)
				{
					if (imm % 4 != 0)
					{
						assembler_error = -1; yyerror("the offset has to be a multiple of 4");
						return;
					}
					imm = imm / 4;
					if (imm >= 0b1 << 8)
					{
						assembler_error = -1; yyerror("offset too large");
						return;
					}
					if (reg2 == 13)
					{
						if (l)
						{
							write |= 0b10011 << 11;
						}
						else
						{
							write |= 0b1001 << 12;
						}
						write |= reg1 << 8;
						write |= imm;
						
						section_write(current_section,&write,2,-1);
						return;
					}
					if (reg2 == 15 && l)
					{
						write |= 0b1001 << 11;
						write |= reg1 << 8;
						write |= imm;
						
						section_write(current_section,&write,2,-1);
						return;
					}
				}
				assembler_error = -1; yyerror("thumb instructions can only use low registers, with some exceptions");
				return;
			}
		}
		
		
		if (! b)
		{
			if (imm % 4 != 0)
			{
				assembler_error = -1; yyerror("the offset has to be a multiple of 4");
				return;
			}
			imm = imm / 4;
		}
		else
		{
			write |= 1 << 12;
		}
		
		if (imm >= 0b1 << 5)
		{
			assembler_error = -1; yyerror("offset too large");
			return;
		}
		
		write |= 0b11 << 13;
		if (l)
		{
			write |= 1 << 11;
		}
		
		write |= imm << 6;
		write |= reg2 << 3;
		write |= reg1;
		
		section_write(current_section,&write,2,-1);
		return;
	}
	assembler_error = -1; yyerror("unsupported instruction");
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
					assembler_error = -1; yyerror("the base register is always updated in post-indexed addressing");
					return;
				}
				write |= 1 << 21;
			}
		}
		else
		{
			if (addressing != post_indexed_addressing_mode)
			{
				assembler_error = -1; yyerror("*t instructions only work with post-indexed access");
				return;
			}
			write |= 1 << 21;
		}
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		uint16_t write = 0;
		if (flags != ALWAYS)
		{
			assembler_error = -1; yyerror("thumb instructions are unconditional");
			return;
		}
		if (update_reg)
		{
			assembler_error = -1; yyerror("the base register can't be updated in thumb");
			return;
		}
		if (addressing == post_indexed_addressing_mode)
		{
			assembler_error = -1; yyerror("thumb doesn't support post-indexed addressing");
			return;
		}
		if (reg1 >= 0b1000 || reg2 >= 0b1000 || reg3 >= 0b1000)
		{
			assembler_error = -1; yyerror("thumb instructions can only use low registers, with some exceptions");
			return;
		}
		
		
		write |= 0b101 << 12;
		if (l)
		{
			write |= 1 << 11;
		}
		if (b)
		{
			write |= 1 << 10;
		}
		write |= reg3 << 6;
		write |= reg2 << 3;
		write |= reg1;
		
		
		section_write(current_section,&write,2,-1);
		return;
	}
	assembler_error = -1; yyerror("unsupported instruction");
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
			assembler_error = -1; yyerror("immediate shift is too big");
			return;
		}
		if (shift_type == 0b11 && shift_val == 0)
		{
			assembler_error = -1; yyerror("ror #0 is not supported");
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
					assembler_error = -1; yyerror("the base register is always updated in post-indexed addressing");
					return;
				}
				write |= 1 << 21;
			}
		}
		else
		{
			if (addressing != post_indexed_addressing_mode)
			{
				assembler_error = -1; yyerror("*t instructions only work with post-indexed access");
				return;
			}
			write |= 1 << 21;
		}
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		assembler_error = -1; yyerror("thumb doesn't support scaled registers");
		return;
	}
	assembler_error = -1; yyerror("unsupported instruction");
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
			assembler_error = -1; yyerror("immediate value can't be < 0");
			return;
		}
		uint8_t rot_imm = 0;
		int rot = is_rotated_imm(imm,&rot_imm);
		if (rot == -1)
		{
			assembler_error = -1; yyerror("immediate value cannot be rotated into a 8 bit constant");
			return;
		}
		write |= rot_imm;
		write |= rot << 8;
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		uint16_t write = 0;
		if (flags != ALWAYS)
		{
			assembler_error = -1; yyerror("thumb data-processing instructions are unconditional");
			return;
		}
		if (opcode == 0b1010) // cmp
		{
			if (reg >= 0b1000)
			{
				assembler_error = -1; yyerror("thumb cmp immediate only supports low registers");
				return;
			}
			else
			{
				if (imm < 0)
				{
					assembler_error = -1; yyerror("immediate value can't be < 0");
					return;
				}
				if (imm >= 1 << 8)
				{
					assembler_error = -1; yyerror("immediate value too big");
					return;
				}
				
				write |= 0b101 << 11;
				write |= reg << 8;
				write |= imm;
			}
			section_write(current_section,&write,2,-1);
			return;
		}
	}
	assembler_error = -1; yyerror("unsupported instruction");
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
			assembler_error = -1; yyerror("immediate shift is too big");
			return;
		}
		if (shift_type == 0b111)
		{
			assembler_error = -1; yyerror("rrx is not supported for immediate shifts");
			return;
		}
		if (shift_type == 0b11 && shift_val == 0)
		{
			assembler_error = -1; yyerror("ror #0 is not supported");
			return;
		}
		write |= shift_val << 7;
		write |= shift_type << 5;
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		uint16_t write = 0;
		if (flags != ALWAYS)
		{
			assembler_error = -1; yyerror("thumb data-processing instructions are unconditional");
			return;
		}
		if (shift_type != 0 || shift_val != 0)
		{
			assembler_error = -1; yyerror("thumb instructions don't support shifts, use a shift instruction instead");
			return;
		}
		
		if (opcode == 0b1001) // teq
		{
			assembler_error = -1; yyerror("thumb doesn't support teq");
			return;
		}
		if (opcode == 0b1010) // cmp
		{
			if (reg1 >= 0b1000 || reg2 >= 0b1000)
			{
				write |= 1 << 14;
				write |= 0b101 << 8;
				if (reg1 >= 0b1000)
				{
					write |= 1 << 7;
					reg1 -= 0b1000;
				}
				if (reg2 >= 0b1000)
				{
					write |= 1 << 6;
					reg2 -= 0b1000;
				}
				write |= reg1;
				write |= reg2 << 3;
			}
			else
			{
				write |= 1 << 14;
				write |= 0b1010 << 6;
				write |= reg1;
				write |= reg2 << 3;
			}
			section_write(current_section,&write,2,-1);
			return;
		}
		
		
		
		if (reg1 >= 0b1000 || reg2 >= 0b1000)
		{
			assembler_error = -1; yyerror("thumb data-processing instructions can only use low registers, with some exceptions");
			return;
		}
		write |= 1 << 14;
		write |= opcode << 6;
		write |= reg1;
		write |= reg2 << 3;
		
		section_write(current_section,&write,2,-1);
		return;
	}
	assembler_error = -1; yyerror("unsupported instruction");
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
		assembler_error = -1; yyerror("thumb instructions don't support shifts. Use a shift instruction instead");
		return;
	}
	assembler_error = -1; yyerror("unsupported instruction");
}



void assemble_data_proc_reg_imm(uint32_t opcode,uint8_t flags,uint8_t update_flags,int64_t reg,int64_t imm)
{
	if (arm)
	{
		if (opcode >= 0b10000)
		{
			assembler_error = -1; yyerror("instruction only available in thumb");
			return;
		}
		if (opcode != 0b1101 && opcode != 0b1111)
		{
			assembler_error = -1; yyerror("instruction needs 3 operands");
			return;
		}
		uint32_t write = 0;
		write |= flags << 28;
		write |= 1 << 25; // immediate form
		write |= opcode << 21;
		write |= update_flags << 20;
		write |= reg << 12;
		
		if (imm < 0)
		{
			assembler_error = -1; yyerror("immediate value can't be 0");
			return;
		}
		uint8_t rot_imm = 0;
		int rot = is_rotated_imm(imm,&rot_imm);
		if (rot == -1)
		{
			assembler_error = -1; yyerror("immediate value cannot be rotated into a 8 bit constant");
			return;
		}
		write |= rot_imm;
		write |= rot << 8;
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		uint16_t write = 0;
		
		switch (opcode)
		{
		case 0b0100:
			if (reg >= 0b1000)
			{
				if (reg == 13) // sp
				{
					if (imm < 0)
					{
						assembler_error = -1; yyerror("immediate value needs to be positive");
						return;
					}
					if (imm % 4 != 0)
					{
						assembler_error = -1; yyerror("immediate value needs to be a multiple of 4");
						return;
					}
					imm = imm / 4;
					if (imm >= 1 << 7)
					{
						assembler_error = -1; yyerror("immediate value too big");
						return;
					}
					write |= 0b1011 << 12;
					write |= imm; // already divided by 4
				}
				else
				{
					assembler_error = -1; yyerror("invalid register");
					return;
				}
			}
			else
			{
				write |= 0b11 << 12;
				write |= reg << 8;
				if (imm < 0)
				{
					assembler_error = -1; yyerror("immediate value needs to be positive");
					return;
				}
				if (imm >= 1 << 8)
				{
					assembler_error = -1; yyerror("immediate value too big");
					return;
				}
				write |= imm;
			}
			section_write(current_section,&write,2,-1);
			return;
		case 0b0010: // sub
			if (reg >= 0b1000)
			{
				if (reg == 13) // sp
				{
					if (imm < 0)
					{
						assembler_error = -1; yyerror("immediate value needs to be positive");
						return;
					}
					if (imm % 4 != 0)
					{
						assembler_error = -1; yyerror("immediate value needs to be a multiple of 4");
						return;
					}
					imm = imm / 4;
					if (imm >= 1 << 7)
					{
						assembler_error = -1; yyerror("immediate value too big");
						return;
					}
					write |= 0b1011 << 12;
					write |= 1 << 7;
					write |= imm; // already divided by 4
				}
				else
				{
					assembler_error = -1; yyerror("invalid register");
					return;
				}
			}
			else
			{
				write |= 0b111 << 11;
				write |= reg << 8;
				if (imm < 0)
				{
					assembler_error = -1; yyerror("immediate value needs to be positive");
					return;
				}
				if (imm >= 1 << 8)
				{
					assembler_error = -1; yyerror("immediate value too big");
					return;
				}
				write |= imm;
			}
			section_write(current_section,&write,2,-1);
			return;
		case 0b1101: // mov
			if (reg >= 0b1000)
			{
				assembler_error = -1; yyerror("instruction needs low register");
				return;
			}
			write |= 1 << 13;
			write |= reg << 8;
			if (imm < 0)
			{
				assembler_error = -1; yyerror("immediate value needs to be positive");
				return;
			}
			if (imm >= 1 << 8)
			{
				assembler_error = -1; yyerror("immediate value too big");
				return;
			}
			write |= imm;
			section_write(current_section,&write,2,-1);
			return;
		}
	}
	assembler_error = -1; yyerror("unsupported instruction");
}


void assemble_data_proc_reg_reg(uint32_t opcode,uint8_t flags,uint8_t update_flags,int64_t reg1,int64_t reg2)
{
	if (arm)
	{
		if (opcode >= 0b10000)
		{
			assembler_error = -1; yyerror("instruction only available in thumb");
			return;
		}
		if (opcode != 0b1101 && opcode != 0b1111)
		{
			assembler_error = -1; yyerror("instruction needs 3 operands");
			return;
		}
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
		uint16_t write = 0;
		
		if (flags != ALWAYS)
		{
			assembler_error = -1; yyerror("thumb data-processing instructions are unconditional");
			return;
		}
		
		if (update_flags)
		{
			assembler_error = -1; yyerror("thumb data-processing instructions always update the flags, except when using high registers");
			return;
		}
		
		switch (opcode)
		{
		case 0b0100: // add
			if (reg1 >= 0b1000 || reg2 >= 0b1000)
			{
				write |= 1 << 14;
				write |= 1 << 10;
				if (reg1 >= 0b1000)
				{
					write |= 1 << 7;
					reg1 -= 0b1000;
				}
				if (reg2 >= 0b1000)
				{
					write |= 1 << 6;
					reg2 -= 0b1000;
				}
				write |= reg1;
				write |= reg2 << 3;
			}
			else
			{
				assembler_error = -1; yyerror("thumb add with 2 registers is only supported if one of them is high. use add ra, ra, rb instead");
				return;
			}
			section_write(current_section,&write,2,-1);
			return;
		case 0b0010: // sub
			assembler_error = -1; yyerror("thumb sub needs 3 low registers");
			return;
		case 0b0011: // rsb
			assembler_error = -1; yyerror("rsb is not available in thumb");
			return;
		case 0b0111: // rsc
			assembler_error = -1; yyerror("rsc is not available in thumb");
			return;
		case 0b1101: // mov (add rd, rn, #0 for low registers)
			if (reg1 >= 0b1000 || reg2 >= 0b1000)
			{
				write |= 1 << 14;
				write |= 0b11 << 9;
				if (reg1 >= 0b1000)
				{
					write |= 1 << 7;
					reg1 -= 0b1000;
				}
				if (reg2 >= 0b1000)
				{
					write |= 1 << 6;
					reg2 -= 0b1000;
				}
				write |= reg1;
				write |= reg2 << 3;
			}
			else
			{
				write |= 0b111 << 10;
				write |= reg1;
				write |= reg2 << 3;
			}
			section_write(current_section,&write,2,-1);
			return;
		case 0b10000: // asr
			opcode = 0b100;
			break;
		case 0b10001: // lsl
			opcode = 0b10;
			break;
		case 0b10010: // lsr
			opcode = 0b11;
			break;
		case 0b10011: // neg
			opcode = 0b1001;
			break;
		case 0b10100: // ror
			opcode = 0b111;
			break;
		}
		
		if (reg1 >= 0b1000 || reg2 >= 0b1000)
		{
			assembler_error = -1; yyerror("thumb data-processing instructions can only use low registers, with some exceptions");
			return;
		}
		
		write |= 1 << 14;
		write |= opcode << 6;
		write |= reg1;
		write |= reg2 << 3;
		
		section_write(current_section,&write,2,-1);
		return;
	}
	assembler_error = -1; yyerror("unsupported instruction");
}

void assemble_data_proc_reg_reg_shift(uint32_t opcode,uint8_t flags,uint8_t update_flags,int64_t reg1,int64_t reg2,uint8_t shift_type,uint8_t shift_val)
{
	if (arm)
	{
		if (opcode >= 0b10000)
		{
			assembler_error = -1; yyerror("instruction only available in thumb");
			return;
		}
		uint32_t write = 0;
		write |= flags << 28;
		write |= opcode << 21;
		write |= update_flags << 20;
		write |= reg1 << 12;
		write |= reg2;
		if (shift_val >= 0b100000)
		{
			assembler_error = -1; yyerror("immediate shift is too big");
			return;
		}
		if (shift_type == 0b111)
		{
			assembler_error = -1; yyerror("rrx is not supported for immediate shifts");
			return;
		}
		if (shift_type == 0b11 && shift_val == 0)
		{
			assembler_error = -1; yyerror("ror #0 is not supported");
			return;
		}
		write |= shift_val << 7;
		write |= shift_type << 5;
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		assembler_error = -1; yyerror("thumb instructions don't support shifts. Use a shift instruction instead");
		return;
	}
	assembler_error = -1; yyerror("unsupported instruction");
}

void assemble_data_proc_reg_reg_shift_reg(uint32_t opcode,uint8_t flags,uint8_t update_flags,int64_t reg1,int64_t reg2,uint8_t shift_type,uint8_t shift_reg)
{
	if (arm)
	{
		if (opcode >= 0b10000)
		{
			assembler_error = -1; yyerror("instruction only available in thumb");
			return;
		}
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
		assembler_error = -1; yyerror("thumb instructions don't support shifts. Use a shift instruction instead");
		return;
	}
	assembler_error = -1; yyerror("unsupported instruction");
}


void assemble_data_proc_reg_reg_imm(uint32_t opcode,uint8_t flags,uint8_t update_flags,int64_t reg1, int64_t reg2,int64_t imm)
{
	if (arm)
	{
		if (opcode >= 0b10000)
		{
			assembler_error = -1; yyerror("instruction only available in thumb");
			return;
		}
		uint32_t write = 0;
		write |= flags << 28;
		write |= 1 << 25; // immediate form
		write |= opcode << 21;
		write |= update_flags << 20;
		write |= reg1 << 12;
		write |= reg2 << 16;
		
		if (imm < 0)
		{
			assembler_error = -1; yyerror("immediate value can't be < 0");
			return;
		}
		uint8_t rot_imm = 0;
		int rot = is_rotated_imm(imm,&rot_imm);
		if (rot == -1)
		{
			assembler_error = -1; yyerror("immediate value cannot be rotated into a 8 bit constant");
			return;
		}
		write |= rot_imm;
		write |= rot << 8;
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		uint16_t write = 0;
		if (flags != ALWAYS)
		{
			assembler_error = -1; yyerror("thumb data-processing instructions are unconditional");
			return;
		}
		if (update_flags)
		{
			assembler_error = -1; yyerror("thumb data-processing instructions always update the flags, except when using high registers");
			return;
		}
		if (imm < 0)
		{
			assembler_error = -1; yyerror("immediate value can't be < 0");
			return;
		}
		if (opcode == 0b0100) // add
		{
			if (reg1 >= 0b1000 || reg2 >= 0b1000)
			{
				if (reg1 >= 0b1000)
				{
					assembler_error = -1; yyerror("first register has to be low");
					return;
				}
				if (reg2 == 15)
				{
					if (imm % 4 != 0)
					{
						assembler_error = -1; yyerror("immediate value has to be a multiple of 4");
						return;
					}
					if (imm/4 >= 0b1 << 8)
					{
						assembler_error = -1; yyerror("immediate value too big");
						return;
					}
					write |= 0b101 << 13;
					write |= reg1 << 8;
					write |= imm/4;
					section_write(current_section,&write,2,-1);
					return;
				}
				if (reg2 == 13)
				{
					if (imm % 4 != 0)
					{
						assembler_error = -1; yyerror("immediate value has to be a multiple of 4");
						return;
					}
					if (imm/4 >= 0b1 << 8)
					{
						assembler_error = -1; yyerror("immediate value too big");
						return;
					}
					write |= 0b10101 << 11;
					write |= reg1 << 8;
					write |= imm/4;
					section_write(current_section,&write,2,-1);
					return;
				}
				assembler_error = -1; yyerror("invalid second register");
				return;
			}
			else
			{
				if (imm >= 1 << 3)
				{
					assembler_error = -1; yyerror("immediate value too big");
					return;
				}
				write |= 0b111 << 10;
				write |= imm << 6;
				write |= reg2 << 3;
				write |= reg1;
			}
			section_write(current_section,&write,2,-1);
			return;
		}
		if (opcode == 0b0010) // sub
		{
			if (reg1 >= 0b1000 || reg2 >= 0b1000)
			{
				assembler_error = -1; yyerror("thumb sub immediate only supports low registers");
				return;
			}
			else
			{
				if (imm >= 1 << 3)
				{
					assembler_error = -1; yyerror("immediate value too big");
					return;
				}
				write |= 0b1111 << 9;
				write |= imm << 6;
				write |= reg2 << 3;
				write |= reg1;
			}
			section_write(current_section,&write,2,-1);
			return;
		}
		if (opcode == 0b10000) // asr
		{
			if (imm == 0)
			{
				assembler_error = -1; yyerror("immediate value can't be 0");
				return;
			}
			if (imm == 1 << 5)
			{
				imm = 0;
			}
			if (imm >= 1 << 5)
			{
				assembler_error = -1; yyerror("immediate value too big");
				return;
			}
			write |= 1 << 12;
			
			write |= reg1;
			write |= reg2 << 3;
			write |= imm << 6;
			section_write(current_section,&write,2,-1);
			return;
		}
		if (opcode == 0b10001) // lsl
		{
			if (imm >= 1 << 5)
			{
				assembler_error = -1; yyerror("immediate value too big");
				return;
			}
			
			write |= reg1;
			write |= reg2 << 3;
			write |= imm << 6;
			section_write(current_section,&write,2,-1);
			return;
		}
		if (opcode == 0b10010) // lsr
		{
			if (imm == 0)
			{
				assembler_error = -1; yyerror("immediate value can't be 0");
				return;
			}
			if (imm == 1 << 5)
			{
				imm = 0;
			}
			if (imm >= 1 << 5)
			{
				assembler_error = -1; yyerror("immediate value too big");
				return;
			}
			write |= 1 << 11;
			
			write |= reg1;
			write |= reg2 << 3;
			write |= imm << 6;
			section_write(current_section,&write,2,-1);
			return;
		}
	}
	assembler_error = -1; yyerror("unsupported instruction");
}


void assemble_data_proc_reg_reg_reg(uint32_t opcode,uint8_t flags,uint8_t update_flags,int64_t reg1,int64_t reg2,int64_t reg3) // this is a special case of a register with immediate shift, which is 0 in this case
{
	assemble_data_proc_reg_reg_reg_shift(opcode,flags,update_flags,reg1,reg2,reg3,0,0);
}


void assemble_data_proc_reg_reg_reg_shift(uint32_t opcode,uint8_t flags,uint8_t update_flags,int64_t reg1,int64_t reg2,int64_t reg3,uint8_t shift_type,uint8_t shift_val)
{
	if (arm)
	{
		if (opcode >= 0b10000)
		{
			assembler_error = -1; yyerror("instruction only available in thumb");
			return;
		}
		uint32_t write = 0;
		write |= flags << 28;
		write |= opcode << 21;
		write |= update_flags << 20;
		write |= reg1 << 12;
		write |= reg2 << 16;
		write |= reg3;
		if (shift_val >= 0b100000)
		{
			assembler_error = -1; yyerror("immediate shift is too big");
			return;
		}
		if (shift_type == 0b111)
		{
			assembler_error = -1; yyerror("rrx is not supported for immediate shifts");
			return;
		}
		if (shift_type == 0b11 && shift_val == 0)
		{
			assembler_error = -1; yyerror("ror #0 is not supported");
			return;
		}
		write |= shift_val << 7;
		write |= shift_type << 5;
		
		section_write(current_section,&write,4,-1);
		return;
	}
	else
	{
		uint16_t write = 0;
		if (flags != ALWAYS)
		{
			assembler_error = -1; yyerror("thumb data-processing instructions are unconditional");
			return;
		}
		if (update_flags)
		{
			assembler_error = -1; yyerror("thumb data-processing instructions always update the flags, except when using high registers");
			return;
		}
		if (shift_type != 0 || shift_val != 0)
		{
			assembler_error = -1; yyerror("thumb instructions don't support shifts, use a shift instruction instead");
			return;
		}
		if (reg1 >= 0b1000 || reg2 >= 0b1000 || reg3 >= 0b1000)
		{
			assembler_error = -1; yyerror("thumb data-processing instructions can only use low registers, with some exceptions");
			return;
		}
		if (opcode == 0b0100) // add
		{
			write |= 0b11 << 11;
			write |= reg1;
			write |= reg2 << 3;
			write |= reg3 << 6;
			section_write(current_section,&write,2,-1);
			return;
		}
		if (opcode == 0b0010) // sub
		{
			write |= 0b11 << 11;
			write |= 1 << 9;
			write |= reg1;
			write |= reg2 << 3;
			write |= reg3 << 6;
			section_write(current_section,&write,2,-1);
			return;
		}
	}
	assembler_error = -1; yyerror("unsupported instruction");
}

void assemble_data_proc_reg_reg_reg_shift_reg(uint32_t opcode,uint8_t flags,uint8_t update_flags,int64_t reg1,int64_t reg2,int64_t reg3,uint8_t shift_type,uint8_t shift_reg)
{
	if (arm)
	{
		if (opcode >= 0b10000)
		{
			assembler_error = -1; yyerror("instruction only available in thumb");
			return;
		}
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
		assembler_error = -1; yyerror("thumb instructions don't support shifts. Use a shift instruction instead");
		return;
	}
	assembler_error = -1; yyerror("unsupported instruction");
}




// frees all labels, fixups and sections
void free_data()
{
	if (sections != NULL)
	{
		for (uint32_t i = 0;i<sections_size;i++)
		{
			if (sections[i] != NULL)
			{
				if (sections[i]->data != NULL)
				{
					free(sections[i]->data);
					sections[i]-> data = NULL;
				}
				sections[i]->size = 0;
				if (sections[i]->name != NULL)
				{
					free(sections[i]->name);
					sections[i]->name = NULL;
				}
				free(sections[i]);
				sections[i] = NULL;
			}
		}
		free(sections);
		sections = NULL;
		sections_size = 0;
		next_section = 0;
	}
	if (labels != NULL)
	{
		for (uint32_t i = 0;i<labels_size;i++)
		{
			if (labels[i] != NULL)
			{
				if (labels[i]->name != NULL)
				{
					free(labels[i]->name);
					labels[i]->name = NULL;
				}
				free(labels[i]);
				labels[i] = NULL;
			}
		}
		free(labels);
		labels = NULL;
		labels_size = 0;
		next_label = 0;
	}
	if (fixups != NULL)
	{
		for (uint32_t i = 0;i<fixups_size;i++)
		{
			if (fixups[i] != NULL)
			{
				if (fixups[i]->name != NULL)
				{
					free(fixups[i]->name);
				}
				fixups[i]->name = NULL;
				free(fixups[i]);
				fixups[i] = NULL;
			}
		}
		free(fixups);
		fixups = NULL;
		fixups_size = 0;
		next_fixup = 0;
	}
}


/*
// IMPORTANT: the malloc wrapper doesn't catch the malloc callst made from inside libc, so strduped memory isn't counted
void* __real_malloc(size_t);
void __real_free(void*);

int allocations = 0;
int max_allocations = 0;
void* __wrap_malloc(size_t size)
{
	allocations++;
	if (allocations > max_allocations)
	{
		max_allocations = allocations;
	}
	void* p = __real_malloc(size);
	if (p == NULL)
	{
		allocations--;
	}
	//printf("allocation size: %d, p: %p\n",size,p);
	printf("allocation size: %d\n",size);
	return p;
}
void __wrap_free(void* p)
{
	//if (p == NULL)
	//{
	//	puts("NULL freed!\n");
	//}
	allocations--;
	__real_free(p);
	//printf("free: %p\n",p);
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






