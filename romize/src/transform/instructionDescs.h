#ifndef INSTRUCTIONDESCS_H
#define INSTRUCTIONDESCS_H

typedef struct instructionEntry {
    char *name;
    UINT8 instructionLength;     /* length of a java instruction, including the instruction itself */
    UINT8 wideInstructionLength; /* length in bytes if modified by a wide opcode, not including the one-byte wide opcode itself */
} instructionEntry, *INSTRUCTION_ENTRY;

extern instructionEntry instructionInfo[256];


#define getInstructionLength(opcode) (instructionInfo[opcode].instructionLength)
#define getWideInstructionLength(opcode) (instructionInfo[opcode].wideInstructionLength)
#define getInstructionName(opcode) (instructionInfo[opcode].name)




#endif
