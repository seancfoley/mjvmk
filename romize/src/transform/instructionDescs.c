
#include "interpret/instructions.h"
#include "instructionDescs.h"



#define VARIABLE_INSTRUCTION_LENGTH 0xfd 
#define UNUSED_INSTRUCTION_LENGTH 0xfe
#define NO_WIDE_INSTRUCTION 0xff

/* This array can be indexed by java opcodes.
 * The first column gives a string representation of the opcode.
 * It gives instruction lengths in byte codes in the 2nd column.
 * It gives the instruction length in byte codes, if the instruction is modified by a wide instruction, 
 * in the 3rd column, not including the one-byte wide opcode itself.
 *
 * Examples:
 * instructionLength[ALOAD_JAVA][1] == 2 (the java ALOAD instruction comprises 2 bytecodes)
 * instructionLength[IINC][1] == 5 (the java IINC instruction comprises 2 bytecodes)
 * instructionLength[IINC][2] == 5 (the java wide IINC instruction comprises 5 bytecodes, not including the WIDE opcode)
 *
 * Note that the length of lookupswitch and 
 * tableswitch instructions will be computed at runtime
 *
 */
instructionEntry instructionInfo[256] = {
    {"nop", 1, NO_WIDE_INSTRUCTION}, /* 0x00 nop */
    {"aconst_null", 1, NO_WIDE_INSTRUCTION}, /* 0x01 aconst_null */
    {"iconst_m1", 1, NO_WIDE_INSTRUCTION}, /* 0x02 iconst_m1 */
    {"iconst_0", 1, NO_WIDE_INSTRUCTION}, /* 0x03 iconst_0 */
    {"iconst_1", 1, NO_WIDE_INSTRUCTION}, /* 0x04 iconst_1 */
    {"iconst_2", 1, NO_WIDE_INSTRUCTION}, /* 0x05 iconst_2 */
    {"iconst_3", 1, NO_WIDE_INSTRUCTION}, /* 0x06 iconst_3 */
    {"iconst_4", 1, NO_WIDE_INSTRUCTION}, /* 0x07 iconst_4 */
    {"iconst_5", 1, NO_WIDE_INSTRUCTION}, /* 0x08 iconst_5 */
    {"lconst_0", 1, NO_WIDE_INSTRUCTION}, /* 0x09 lconst_0 */
    {"lconst_1", 1, NO_WIDE_INSTRUCTION}, /* 0x0a lconst_1 */
    {"fconst_0", 1, NO_WIDE_INSTRUCTION}, /* 0x0b fconst_0 */
    {"fconst_1", 1, NO_WIDE_INSTRUCTION}, /* 0x0c fconst_1 */
    {"fconst_2", 1, NO_WIDE_INSTRUCTION}, /* 0x0d fconst_2 */
    {"dconst_0", 1, NO_WIDE_INSTRUCTION}, /* 0x0e dconst_0 */
    {"dconst_1", 1, NO_WIDE_INSTRUCTION}, /* 0x0f dconst_1 */
    {"bipush", 2, NO_WIDE_INSTRUCTION}, /* 0x10 bipush */
    {"sipush", 3, NO_WIDE_INSTRUCTION}, /* 0x11 sipush */
    {"ldc", 2, NO_WIDE_INSTRUCTION}, /* 0x12 ldc */
    {"ldc_w", 3, NO_WIDE_INSTRUCTION}, /* 0x13 ldc_w */
    {"ldc2_w", 3, NO_WIDE_INSTRUCTION}, /* 0x14 ldc2_w */
    {"iload", 2, 3},     /* 0x15 iload */
    {"lload", 2, 3},     /* 0x16 lload */
    {"fload", 2, 3},     /* 0x17 fload */
    {"dload", 2, 3},     /* 0x18 dload */
    {"aload", 2, 3},     /* 0x19 aload */
    {"iload_0", 1, NO_WIDE_INSTRUCTION}, /* 0x1a iload_0 */
    {"iload_1", 1, NO_WIDE_INSTRUCTION}, /* 0x1b iload_1 */
    {"iload_2", 1, NO_WIDE_INSTRUCTION}, /* 0x1c iload_2 */
    {"iload_3", 1, NO_WIDE_INSTRUCTION}, /* 0x1d iload_3 */
    {"lload_0", 1, NO_WIDE_INSTRUCTION}, /* 0x1e lload_0 */
    {"lload_1", 1, NO_WIDE_INSTRUCTION}, /* 0x1f lload_1 */
    {"lload_2", 1, NO_WIDE_INSTRUCTION}, /* 0x20 lload_2 */
    {"lload_3", 1, NO_WIDE_INSTRUCTION}, /* 0x21 lload_3 */
    {"fload_0", 1, NO_WIDE_INSTRUCTION}, /* 0x22 fload_0 */
    {"fload_1", 1, NO_WIDE_INSTRUCTION}, /* 0x23 fload_1 */
    {"fload_2", 1, NO_WIDE_INSTRUCTION}, /* 0x24 fload_2 */
    {"fload_3", 1, NO_WIDE_INSTRUCTION}, /* 0x25 fload_3 */
    {"dload_0", 1, NO_WIDE_INSTRUCTION}, /* 0x26 dload_0 */
    {"dload_1", 1, NO_WIDE_INSTRUCTION}, /* 0x27 dload_1 */
    {"dload_2", 1, NO_WIDE_INSTRUCTION}, /* 0x28 dload_2 */
    {"dload_3", 1, NO_WIDE_INSTRUCTION}, /* 0x29 dload_3 */
    {"aload_0", 1, NO_WIDE_INSTRUCTION}, /* 0x2a aload_0 */
    {"aload_1", 1, NO_WIDE_INSTRUCTION}, /* 0x2b aload_1 */
    {"aload_2", 1, NO_WIDE_INSTRUCTION}, /* 0x2c aload_2 */
    {"aload_3", 1, NO_WIDE_INSTRUCTION}, /* 0x2d aload_3 */
    {"iaload", 1, NO_WIDE_INSTRUCTION}, /* 0x2e iaload */
    {"laload", 1, NO_WIDE_INSTRUCTION}, /* 0x2f laload */
    {"faload", 1, NO_WIDE_INSTRUCTION}, /* 0x30 faload */
    {"daload", 1, NO_WIDE_INSTRUCTION}, /* 0x31 daload */
    {"aaload", 1, NO_WIDE_INSTRUCTION}, /* 0x32 aaload */
    {"baload", 1, NO_WIDE_INSTRUCTION}, /* 0x33 baload */
    {"caload", 1, NO_WIDE_INSTRUCTION}, /* 0x34 caload */
    {"saload", 1, NO_WIDE_INSTRUCTION}, /* 0x35 saload */
    {"istore", 2, 3},     /* 0x36 istore */
    {"lstore", 2, 3},     /* 0x37 lstore */
    {"fstore", 2, 3},     /* 0x38 fstore */
    {"dstore", 2, 3},     /* 0x39 dstore */
    {"astore", 2, 3},     /* 0x3a astore */
    {"istore_0", 1, NO_WIDE_INSTRUCTION}, /* 0x3b istore_0 */
    {"istore_1", 1, NO_WIDE_INSTRUCTION}, /* 0x3c istore_1 */
    {"istore_2", 1, NO_WIDE_INSTRUCTION}, /* 0x3d istore_2 */
    {"istore_3", 1, NO_WIDE_INSTRUCTION}, /* 0x3e istore_3 */
    {"lstore_0", 1, NO_WIDE_INSTRUCTION}, /* 0x3f lstore_0 */
    {"lstore_1", 1, NO_WIDE_INSTRUCTION}, /* 0x40 lstore_1 */
    {"lstore_2", 1, NO_WIDE_INSTRUCTION}, /* 0x41 lstore_2 */
    {"lstore_3", 1, NO_WIDE_INSTRUCTION}, /* 0x42 lstore_3 */
    {"fstore_0", 1, NO_WIDE_INSTRUCTION}, /* 0x43 fstore_0 */
    {"fstore_1", 1, NO_WIDE_INSTRUCTION}, /* 0x44 fstore_1 */
    {"fstore_2", 1, NO_WIDE_INSTRUCTION}, /* 0x45 fstore_2 */
    {"fstore_3", 1, NO_WIDE_INSTRUCTION}, /* 0x46 fstore_3 */
    {"dstore_0", 1, NO_WIDE_INSTRUCTION}, /* 0x47 dstore_0 */
    {"dstore_1", 1, NO_WIDE_INSTRUCTION}, /* 0x48 dstore_1 */
    {"dstore_2", 1, NO_WIDE_INSTRUCTION}, /* 0x49 dstore_2 */
    {"dstore_3", 1, NO_WIDE_INSTRUCTION}, /* 0x4a dstore_3 */
    {"astore_0", 1, NO_WIDE_INSTRUCTION}, /* 0x4b astore_0 */
    {"astore_1", 1, NO_WIDE_INSTRUCTION}, /* 0x4c astore_1 */
    {"astore_2", 1, NO_WIDE_INSTRUCTION}, /* 0x4d astore_2 */
    {"astore_3", 1, NO_WIDE_INSTRUCTION}, /* 0x4e astore_3 */
    {"iastore", 1, NO_WIDE_INSTRUCTION}, /* 0x4f iastore */
    {"lastore", 1, NO_WIDE_INSTRUCTION}, /* 0x50 lastore */
    {"fastore", 1, NO_WIDE_INSTRUCTION}, /* 0x51 fastore */
    {"dastore", 1, NO_WIDE_INSTRUCTION}, /* 0x52 dastore */
    {"aastore", 1, NO_WIDE_INSTRUCTION}, /* 0x53 aastore */
    {"bastore", 1, NO_WIDE_INSTRUCTION}, /* 0x54 bastore */
    {"castore", 1, NO_WIDE_INSTRUCTION}, /* 0x55 castore */
    {"sastore", 1, NO_WIDE_INSTRUCTION}, /* 0x56 sastore */
    {"pop", 1, NO_WIDE_INSTRUCTION}, /* 0x57 pop */
    {"pop2", 1, NO_WIDE_INSTRUCTION}, /* 0x58 pop2 */
    {"dup", 1, NO_WIDE_INSTRUCTION}, /* 0x59 dup */
    {"dup_x1", 1, NO_WIDE_INSTRUCTION}, /* 0x5a dup_x1 */
    {"dup_x2", 1, NO_WIDE_INSTRUCTION}, /* 0x5b dup_x2 */
    {"dup2", 1, NO_WIDE_INSTRUCTION}, /* 0x5c dup2 */
    {"dup2_x1", 1, NO_WIDE_INSTRUCTION}, /* 0x5d dup2_x1 */
    {"dup2_x2", 1, NO_WIDE_INSTRUCTION}, /* 0x5e dup2_x2 */
    {"swap", 1, NO_WIDE_INSTRUCTION}, /* 0x5f swap */
    {"iadd", 1, NO_WIDE_INSTRUCTION}, /* 0x60 iadd */
    {"ladd", 1, NO_WIDE_INSTRUCTION}, /* 0x61 ladd */
    {"fadd", 1, NO_WIDE_INSTRUCTION}, /* 0x62 fadd */
    {"dadd", 1, NO_WIDE_INSTRUCTION}, /* 0x63 dadd */
    {"isub", 1, NO_WIDE_INSTRUCTION}, /* 0x64 isub */
    {"lsub", 1, NO_WIDE_INSTRUCTION}, /* 0x65 lsub */
    {"fsub", 1, NO_WIDE_INSTRUCTION}, /* 0x66 fsub */
    {"dsub", 1, NO_WIDE_INSTRUCTION}, /* 0x67 dsub */
    {"imul", 1, NO_WIDE_INSTRUCTION}, /* 0x68 imul */
    {"lmul", 1, NO_WIDE_INSTRUCTION}, /* 0x69 lmul */
    {"fmul", 1, NO_WIDE_INSTRUCTION}, /* 0x6a fmul */
    {"dmul", 1, NO_WIDE_INSTRUCTION}, /* 0x6b dmul */
    {"idiv", 1, NO_WIDE_INSTRUCTION}, /* 0x6c idiv */
    {"ldiv", 1, NO_WIDE_INSTRUCTION}, /* 0x6d ldiv */
    {"fdiv", 1, NO_WIDE_INSTRUCTION}, /* 0x6e fdiv */
    {"ddiv", 1, NO_WIDE_INSTRUCTION}, /* 0x6f ddiv */
    {"irem", 1, NO_WIDE_INSTRUCTION}, /* 0x70 irem */
    {"lrem", 1, NO_WIDE_INSTRUCTION}, /* 0x71 lrem */
    {"frem", 1, NO_WIDE_INSTRUCTION}, /* 0x72 frem */
    {"drem", 1, NO_WIDE_INSTRUCTION}, /* 0x73 drem */
    {"ineg", 1, NO_WIDE_INSTRUCTION}, /* 0x74 ineg */
    {"lneg", 1, NO_WIDE_INSTRUCTION}, /* 0x75 lneg */
    {"fneg", 1, NO_WIDE_INSTRUCTION}, /* 0x76 fneg */
    {"dneg", 1, NO_WIDE_INSTRUCTION}, /* 0x77 dneg */
    {"ishl", 1, NO_WIDE_INSTRUCTION}, /* 0x78 ishl */
    {"lshl", 1, NO_WIDE_INSTRUCTION}, /* 0x79 lshl */
    {"ishr", 1, NO_WIDE_INSTRUCTION}, /* 0x7a ishr */
    {"lshr", 1, NO_WIDE_INSTRUCTION}, /* 0x7b lshr */
    {"iushr", 1, NO_WIDE_INSTRUCTION}, /* 0x7c iushr */
    {"lushr", 1, NO_WIDE_INSTRUCTION}, /* 0x7d lushr */
    {"iand", 1, NO_WIDE_INSTRUCTION}, /* 0x7e iand */
    {"land", 1, NO_WIDE_INSTRUCTION}, /* 0x7f land */
    {"ior", 1, NO_WIDE_INSTRUCTION}, /* 0x80 ior */
    {"lor", 1, NO_WIDE_INSTRUCTION}, /* 0x81 lor */
    {"ixor", 1, NO_WIDE_INSTRUCTION}, /* 0x82 ixor */
    {"lxor", 1, NO_WIDE_INSTRUCTION}, /* 0x83 lxor */
    {"iinc", 3, 5},     /* 0x84 iinc */
    {"i2l", 1, NO_WIDE_INSTRUCTION}, /* 0x85 i2l */
    {"i2f", 1, NO_WIDE_INSTRUCTION}, /* 0x86 i2f */
    {"i2d", 1, NO_WIDE_INSTRUCTION}, /* 0x87 i2d */
    {"l2i", 1, NO_WIDE_INSTRUCTION}, /* 0x88 l2i */
    {"l2f", 1, NO_WIDE_INSTRUCTION}, /* 0x89 l2f */
    {"l2d", 1, NO_WIDE_INSTRUCTION}, /* 0x8a l2d */
    {"f2i", 1, NO_WIDE_INSTRUCTION}, /* 0x8b f2i */
    {"f2l", 1, NO_WIDE_INSTRUCTION}, /* 0x8c f2l */
    {"f2d", 1, NO_WIDE_INSTRUCTION}, /* 0x8d f2d */
    {"d2i", 1, NO_WIDE_INSTRUCTION}, /* 0x8e d2i */
    {"d2l", 1, NO_WIDE_INSTRUCTION}, /* 0x8f d2l */
    {"d2f", 1, NO_WIDE_INSTRUCTION}, /* 0x90 d2f */
    {"i2b", 1, NO_WIDE_INSTRUCTION}, /* 0x91 i2b */
    {"i2c", 1, NO_WIDE_INSTRUCTION}, /* 0x92 i2c */
    {"i2s", 1, NO_WIDE_INSTRUCTION}, /* 0x93 i2s */
    {"lcmp", 1, NO_WIDE_INSTRUCTION}, /* 0x94 lcmp */
    {"fcmpl", 1, NO_WIDE_INSTRUCTION}, /* 0x95 fcmpl */
    {"fcmpg", 1, NO_WIDE_INSTRUCTION}, /* 0x96 fcmpg */
    {"dcmpl", 1, NO_WIDE_INSTRUCTION}, /* 0x97 dcmpl */
    {"dcmpg", 1, NO_WIDE_INSTRUCTION}, /* 0x98 dcmpg */
    {"ifeq", 3, NO_WIDE_INSTRUCTION}, /* 0x99 ifeq */
    {"ifne", 3, NO_WIDE_INSTRUCTION}, /* 0x9a ifne */
    {"iflt", 3, NO_WIDE_INSTRUCTION}, /* 0x9b iflt */
    {"ifge", 3, NO_WIDE_INSTRUCTION}, /* 0x9c ifge */
    {"ifgt", 3, NO_WIDE_INSTRUCTION}, /* 0x9d ifgt */
    {"ifle", 3, NO_WIDE_INSTRUCTION}, /* 0x9e ifle */
    {"if_icmpeq", 3, NO_WIDE_INSTRUCTION}, /* 0x9f if_icmpeq */
    {"if_icmpne", 3, NO_WIDE_INSTRUCTION}, /* 0xa0 if_icmpne */
    {"if_icmplt", 3, NO_WIDE_INSTRUCTION}, /* 0xa1 if_icmplt */
    {"if_icmpge", 3, NO_WIDE_INSTRUCTION}, /* 0xa2 if_icmpge */
    {"if_icmpgt", 3, NO_WIDE_INSTRUCTION}, /* 0xa3 if_icmpgt */
    {"if_icmple", 3, NO_WIDE_INSTRUCTION}, /* 0xa4 if_icmple */
    {"if_acmpeq", 3, NO_WIDE_INSTRUCTION}, /* 0xa5 if_acmpeq */
    {"if_acmpne", 3, NO_WIDE_INSTRUCTION}, /* 0xa6 if_acmpne */
    {"goto", 3, NO_WIDE_INSTRUCTION}, /* 0xa7 goto  */
    {"jsr", 3, NO_WIDE_INSTRUCTION}, /* 0xa8 jsr */
    {"ret", 2, 3}, /* 0xa9 ret */
    {"tableswitch", VARIABLE_INSTRUCTION_LENGTH, NO_WIDE_INSTRUCTION}, /* 0xaa tableswitch */
    {"lookupswitch", VARIABLE_INSTRUCTION_LENGTH, NO_WIDE_INSTRUCTION}, /* 0xab lookupswitch */
    {"ireturn", 1, NO_WIDE_INSTRUCTION}, /* 0xac ireturn */
    {"lreturn", 1, NO_WIDE_INSTRUCTION}, /* 0xad lreturn */
    {"freturn", 1, NO_WIDE_INSTRUCTION}, /* 0xae freturn */
    {"dreturn", 1, NO_WIDE_INSTRUCTION}, /* 0xaf dreturn */
    {"areturn", 1, NO_WIDE_INSTRUCTION}, /* 0xb0 areturn */
    {"return", 1, NO_WIDE_INSTRUCTION}, /* 0xb1 return */
    {"getstatic", 3, NO_WIDE_INSTRUCTION}, /* 0xb2 getstatic */
    {"putstatic", 3, NO_WIDE_INSTRUCTION}, /* 0xb3 putstatic */
    {"getfield", 3, NO_WIDE_INSTRUCTION}, /* 0xb4 getfield */
    {"putfield", 3, NO_WIDE_INSTRUCTION}, /* 0xb5 putfield */
    {"invokevirtual", 3, NO_WIDE_INSTRUCTION}, /* 0xb6 invokevirtual */
    {"invokespecial", 3, NO_WIDE_INSTRUCTION}, /* 0xb7 invokespecial */
    {"invokestatic", 3, NO_WIDE_INSTRUCTION}, /* 0xb8 invokestatic */
    {"invokeinterface", 5, NO_WIDE_INSTRUCTION}, /* 0xb9 invokeinterface */
    {"xxxunusedxxx1", UNUSED_INSTRUCTION_LENGTH, NO_WIDE_INSTRUCTION}, /* 0xba xxxunusedxxx1 */
    {"new", 3, NO_WIDE_INSTRUCTION}, /* 0xbb new */
    {"newarray", 2, NO_WIDE_INSTRUCTION}, /* 0xbc newarray */
    {"anewarray", 3, NO_WIDE_INSTRUCTION}, /* 0xbd anewarray */
    {"arraylength", 1, NO_WIDE_INSTRUCTION}, /* 0xbe arraylength */
    {"athrow", 1, NO_WIDE_INSTRUCTION}, /* 0xbf athrow */
    {"checkcast", 3, NO_WIDE_INSTRUCTION}, /* 0xc0 checkcast */
    {"instanceof", 3, NO_WIDE_INSTRUCTION}, /* 0xc1 instanceof */
    {"monitorenter", 1, NO_WIDE_INSTRUCTION}, /* 0xc2 monitorenter */
    {"monitorexit", 1, NO_WIDE_INSTRUCTION}, /* 0xc3 monitorexit */
    {"wide", 1, NO_WIDE_INSTRUCTION}, /* 0xc4 wide */
    {"multianewarray", 4, NO_WIDE_INSTRUCTION}, /* 0xc5 multianewarray */
    {"ifnull", 3, NO_WIDE_INSTRUCTION}, /* 0xc6 ifnull */
    {"ifnonnull", 3, NO_WIDE_INSTRUCTION}, /* 0xc7 ifnonnull */
    {"goto_w", 5, NO_WIDE_INSTRUCTION}, /* 0xc8 goto_w */
    {"jsr_w", 5, NO_WIDE_INSTRUCTION}, /* 0xc9 jsr_w */
    {"breakpoint", UNUSED_INSTRUCTION_LENGTH, NO_WIDE_INSTRUCTION}, /* 0xca breakpoint */
    {"invokestatic_quick", 3, NO_WIDE_INSTRUCTION}, /* 0xcb */
    {"invokestatic_quick_native", 3, NO_WIDE_INSTRUCTION}, /* 0xcc */
    {"invokestatic_quick_sync", 3, NO_WIDE_INSTRUCTION}, /* 0xcd */
    {"invokestatic_quick_native_sync", 3, NO_WIDE_INSTRUCTION}, /* 0xce */
    {"invokeinterface_quick", 5, NO_WIDE_INSTRUCTION}, /* 0xcf */
    {"invokespecial_quick", 3, NO_WIDE_INSTRUCTION}, /* 0xd0 */
    {"invokespecial_quick_native", 3, NO_WIDE_INSTRUCTION}, /* 0xd1 */
    {"invokespecial_quick_sync", 3, NO_WIDE_INSTRUCTION}, /* 0xd2 */
    {"invokespecial_quick_native_sync", 3, NO_WIDE_INSTRUCTION}, /* 0xd3 */
    {"invokevirtual_quick", 3, NO_WIDE_INSTRUCTION}, /* 0xd4 */
    {"getstatic_quick_double_primitive", 3, NO_WIDE_INSTRUCTION}, /* 0xd5 */
    {"getstatic_quick_primitive", 3, NO_WIDE_INSTRUCTION}, /* 0xd6 */
    {"getstatic_quick_object", 3, NO_WIDE_INSTRUCTION}, /* 0xd7 */
    {"putstatic_quick_double_primitive", 3, NO_WIDE_INSTRUCTION}, /* 0xd8 */
    {"putstatic_quick_primitive", 3, NO_WIDE_INSTRUCTION}, /* 0xd9 */
    {"putstatic_quick_object", 3, NO_WIDE_INSTRUCTION}, /* 0xda */
    {"getfield_quick_double_primitive", 3, NO_WIDE_INSTRUCTION}, /* 0xdb */
    {"getfield_quick_primitive", 3, NO_WIDE_INSTRUCTION}, /* 0xdc */
    {"getfield_quick_object", 3, NO_WIDE_INSTRUCTION}, /* 0xdd */
    {"putfield_quick_double_primitive", 3, NO_WIDE_INSTRUCTION}, /* 0xde */
    {"putfield_quick_primitive", 3, NO_WIDE_INSTRUCTION}, /* 0xdf */
    {"putfield_quick_object", 3, NO_WIDE_INSTRUCTION}, /* 0xe0 */
    {"return_quick_sync", 1, NO_WIDE_INSTRUCTION}, /* 0xe1 */
    {"ireturn_quick_sync", 1, NO_WIDE_INSTRUCTION}, /* 0xe2 */
    {"lreturn_quick_sync", 1, NO_WIDE_INSTRUCTION}, /* 0xe3 */
    {"areturn_quick_sync", 1, NO_WIDE_INSTRUCTION}, /* 0xe4 */
    {"return_quick_non_sync", 1, NO_WIDE_INSTRUCTION}, /* 0xe5 */
    {"ireturn_quick_non_sync", 1, NO_WIDE_INSTRUCTION}, /* 0xe6 */
    {"lreturn_quick_non_sync", 1, NO_WIDE_INSTRUCTION}, /* 0xe7 */
    {"areturn_quick_non_sync", 1, NO_WIDE_INSTRUCTION}, /* 0xe8 */
    {"checkcast_quick", 3, NO_WIDE_INSTRUCTION}, /* 0xe9 */
    {"instanceof_quick", 3, NO_WIDE_INSTRUCTION}, /* 0xea */
    {"new_quick", 3, NO_WIDE_INSTRUCTION}, /* 0xeb */
    {"anewarray_quick", 3, NO_WIDE_INSTRUCTION}, /* 0xec */
    {"multianewarray_quick", 4, NO_WIDE_INSTRUCTION}, /* 0xed */
    {"newarray_quick", 2, NO_WIDE_INSTRUCTION}, /* 0xee */
    {"getfield_0_quick_primitive", 3, NO_WIDE_INSTRUCTION}, /* 0xef */
    {"string_char_at_quick", 3, NO_WIDE_INSTRUCTION}, /* 0xf0 */
    {"string_char_at_internal_quick", 3, NO_WIDE_INSTRUCTION}, /* 0xf1 */
    {"math_quick", 3, NO_WIDE_INSTRUCTION}, /* 0xf2 */
    {"vector_element_at_internal_quick", 3, NO_WIDE_INSTRUCTION}, /* 0xf3 */
    {"return_address_store", 2, 3}, /* 0xf4 */
    {"return_address_store", 1, NO_WIDE_INSTRUCTION}, /* 0xf5 */
    {"return_address_store", 1, NO_WIDE_INSTRUCTION}, /* 0xf6 */
    {"return_address_store", 1, NO_WIDE_INSTRUCTION}, /* 0xf7 */
    {"return_address_store", 1, NO_WIDE_INSTRUCTION}, /* 0xf8 */
    {"", UNUSED_INSTRUCTION_LENGTH, NO_WIDE_INSTRUCTION}, /* 0xf9 */
    {"", UNUSED_INSTRUCTION_LENGTH, NO_WIDE_INSTRUCTION}, /* 0xfa */
    {"", UNUSED_INSTRUCTION_LENGTH, NO_WIDE_INSTRUCTION}, /* 0xfb */
    {"clinit_opcode", 1, NO_WIDE_INSTRUCTION}, /* 0xfc */
    {"idle_opcode", 1, NO_WIDE_INSTRUCTION}, /* 0xfd */
    {"first_opcode", 1, NO_WIDE_INSTRUCTION}, /* 0xfe impdep1 */
    {"init_opcode", 1, NO_WIDE_INSTRUCTION}, /* 0xff impdep2 */
};






