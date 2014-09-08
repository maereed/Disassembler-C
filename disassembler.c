#include <stdlib.h>
#include <stdio.h>

const char reg[32][6] = {"$zero","$at","$v0","$v1","$a0","$a1","$a2","$a3",
  "$t0","$t1","$t2","$t3","$t4","$t5","$t6","$t7","$s0","$s1","$s2","$s3",
  "$s4","$s5","$s6","$s7","$t8","$t9","$k0","$k1","$gp","$sp","$fp","$ra"};

static void Decode(int pc, int instr)  // do not make any changes outside of this function
{
  int opcode, rs, rt, rd, shamt, funct, uimm, simm, addr;

  opcode = (instr >> 26) & 0x3f;/* start at bit 26 and read in 6 bits (0x3f -- 111111*/
  rs = (instr >> 21) & 0x1f;/* start at bit 21 and read in 5 bits */
  rt = (instr >> 16) & 0x1f;/* start at bit 16 and read in 5 bits */
  rd = (instr >> 11) & 0x1f;/* given */
  shamt = (instr >> 6) & 0x1f;/* start at bit 6 and read in 5 bits for shift amount */
  funct = (instr) & 0x3f;/* start at bit 0 and read in 6 bits */
  uimm = (instr) & 0xffff;/* unsigned immediate start reading at bit 0 and read 16 bits no need to convert */
  simm = ((signed)uimm << 16) >> 16;/* given */
  addr = (instr >> 0) & 0x3ffffff;/* address field for jumps start reading at 0 and grab 26 bits -- 11010 */

  switch (opcode) {
    case 0x00:
      switch (funct) {
        case 0x00: printf("%8x: sll %s, %s, %d\n", pc, reg[rd], reg[rs], shamt); break; /* sll RD, RS, shift5*/
        case 0x03: printf("%8x: sra %s, %s, %d\n", pc, reg[rd], reg[rs], shamt); break; /* sra RD, RS, shift5*/
        case 0x08: printf("%8x: jr %s\n", pc, reg[rs]); break;/* jr */
        case 0x10: printf("%8x: mfhi %s\n", pc, reg[rd]); break; /* mfhi */
        case 0x12: printf("%8x: mflo %s\n", pc, reg[rd]); break;
        case 0x18: printf("%8x: mult %s, %s\n", pc, reg[rs], reg[rt]); break; /* mult */ 
        case 0x1a: printf("%8x: div %s, %s\n", pc, reg[rs], reg[rt]); break;/* div */
        case 0x21: printf("%8x: addu %s, %s, %s\n", pc, reg[rd], reg[rs], reg[rt]); break;
        case 0x23: printf("%8x: subu %s, %s, %s\n", pc, reg[rd], reg[rs], reg[rt]); break;/* subu */
        case 0x2a: printf("%8x: slt %s, %s, %s\n", pc, reg[rd], reg[rs], reg[rt]); break;/* slt */
        default: printf("%8x: unimplemented\n", pc);
      }
      break;
    case 0x02: printf("%8x: j %x\n", pc, ((pc+4) & 0xf0000000) + addr*4); break;/* j */ 
    case 0x03: printf("%8x: jal %x\n", pc, ((pc+4) & 0xf0000000) + addr*4); break;
    case 0x04: printf("%8x: beq %s, %s, %x\n", pc, reg[rs], reg[rt], ((pc+4) & 0xffff) + addr*4); break;/* beq */ 
    case 0x05: /* bne */ 
    case 0x08: printf("%8x: addi %s, %s, %d\n", pc, reg[rt], reg[rs], simm);break;/* addi */ 
    case 0x09: printf("%8x: addiu %s, %s, %d\n", pc, reg[rt], reg[rs], simm);break;/* addiu */ 
    case 0x0c: printf("%8x: andi %s, %s, %d\n", pc, reg[rt], reg[rs], uimm);break;/* andi */ 
    case 0x0f: printf("%8x: lui %s, %d\n", pc, reg[rt], simm); break;
    case 0x1a: printf("%8x: trap %x\n", pc, addr); break;
    case 0x23: /* lw */ 
    case 0x2b: /* sw */ 
    default: printf("%8x: unimplemented\n", pc);
  }
}

static int Convert(unsigned int x)
{
  return (x >> 24) | ((x >> 8) & 0xff00) | ((x << 8) & 0xff0000) | (x << 24);
}

int main(int argc, char *argv[])
{
  int c, count, start, little_endian, *instruction;
  FILE *f;

  printf("CS3339 -- MIPS Disassembler\n");
  if (argc != 2) {fprintf(stderr, "usage: %s mips_executable\n", argv[0]); exit(-1);}
  if (sizeof(int) != 4) {fprintf(stderr, "error: need 4-byte integers\n"); exit(-1);}

  count = 1;
  little_endian = *((char *)&count);

  f = fopen(argv[1], "r+b");
  if (f == NULL) {fprintf(stderr, "error: could not open file %s\n", argv[1]); exit(-1);}
  c = fread(&count, 4, 1, f);
  if (c != 1) {fprintf(stderr, "error: could not read count from file %s\n", argv[1]); exit(-1);}
  if (little_endian) {
    count = Convert(count);
  }
  c = fread(&start, 4, 1, f);
  if (c != 1) {fprintf(stderr, "error: could not read start from file %s\n", argv[1]); exit(-1);}
  if (little_endian) {
    start = Convert(start);
  }

  instruction = (int *)(malloc(count * 4));
  if (instruction == NULL) {fprintf(stderr, "error: out of memory\n"); exit(-1);}
  c = fread(instruction, 4, count, f);
  if (c != count) {fprintf(stderr, "error: could not read instructions from file %s\n", argv[1]); exit(-1);}
  fclose(f);
  if (little_endian) {
    for (c = 0; c < count; c++) {
      instruction[c] = Convert(instruction[c]);
    }
  }

  for (c = 0; c < count; c++) {
    Decode(start + c * 4, instruction[c]);
  }
}

