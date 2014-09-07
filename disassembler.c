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
  uimm = (instr) & 0xffff);/* unsigned immediate... */
  simm = ((signed)uimm << 16) >> 16;
  addr = (instr >> 0) & 0x3ffffff;/* address field for jumps start reading at 0 and grab 26 bits -- 11010 */

  switch (opcode) {
    case 0x00:
      switch (funct) {
        case 0x00: printf("%8x: sll %s, %s(%s)\n", pc, reg[rd], reg[shamt], reg[rs]);/* sll */ break;
        case 0x03: /* sra */ break;
        case 0x08: /* jr */ break;
        case 0x10: /* mfhi */ break;
        case 0x12: printf("%8x: mflo %s\n", pc, reg[rd]); break;
        case 0x18: /* mult */ break;
        case 0x1a: /* div */ break;
        case 0x21: printf("%8x: addu %s, %s, %s\n", pc, reg[rd], reg[rs], reg[rt]); break;
        case 0x23: /* subu */ break;
        case 0x2a: /* slt */ break;
        default: printf("%8x: unimplemented\n", pc);
      }
      break;
    case 0x02: /* j */ break;
    case 0x03: printf("%8x: jal %x\n", pc, ((pc+4) & 0xf0000000) + addr*4); break;
    case 0x04: /* beq */ break;
    case 0x05: /* bne */ break;
    case 0x08: /* addi */ break;
    case 0x09: /* addiu */ break;
    case 0x0c: /* andi */ break;
    case 0x0f: printf("%8x: lui %s, %d\n", pc, reg[rt], simm); break;
    case 0x1a: printf("%8x: trap %x\n", pc, addr); break;
    case 0x23: /* lw */ break;
    case 0x2b: /* sw */ break;
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

