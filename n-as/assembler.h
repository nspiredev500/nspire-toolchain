#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#define ASSEMBLER_SWI_ALLOWED 0b1
#define ASSEMBLER_PSR_ALLOWED (0b1 << 1)
#define ASSEMBLER_COPROCESSOR_ALLOWED (0b1 << 2)


extern char asm_error_msg[200];

int assemble_string(const char* string, uint16_t flags, uint32_t* size, void** mem);












#endif