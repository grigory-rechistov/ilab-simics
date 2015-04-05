# CHIP16 instruction set definition

machine: chip16, 32, LE
group: A1, opcode<24:31>, y<20:23>, x<16:19>, ll<8:15>, hh<0:7>
group: A2, opcode<24:31>, y<20:23>, x<16:19>, clr1<12:15>, z<8:11>, clr2<0:7>

# TODO fill me

instruction: NOP
pattern: opcode == 0
mnemonic: "nop"
/*Nop do nothing*/
endinstruction

instruction: MOV
pattern: opcode == 0x24
mnemonic: "mov r%d, r%d", x, y

decode_data.cpu->chip16_reg[x] = decode_data.cpu->chip16_reg[y];

endinstruction

instruction: SND0
pattern: opcode == 0x09
mnemonic: "snd0"
// snd0 (command word: 0, 2) - stop playing sounds (freq = 0).
chip16_t *core = decode_data.cpu;
chip16_write_memory16 (core, SND_MEM_ADDR, SND_MEM_ADDR, 0x0002);
chip16_write_memory16 (core, SND_MEM_ADDR, SND_MEM_ADDR,      0);
chip16_write_memory16 (core, SND_MEM_ADDR, SND_MEM_ADDR,      0);
endinstruction

instruction: SND1
pattern: opcode == 0x0a
mnemonic: "snd1 0x%x", ((hh << 8) + ll)
// snd1 (command word: 0, 2) - play 500Hz tone for HHLL ms.
chip16_t *core = decode_data.cpu;
chip16_write_memory16 (core, SND_MEM_ADDR, SND_MEM_ADDR, 0x0002);
chip16_write_memory16 (core, SND_MEM_ADDR, SND_MEM_ADDR, 500);
chip16_write_memory16 (core, SND_MEM_ADDR, SND_MEM_ADDR, ((hh << 8) + ll));
endinstruction

instruction: SND2
pattern: opcode == 0x0b
mnemonic: "snd2 0x%x%x", hh, ll
// snd2 (command word: 0, 2) - play 1000Hz tone for HHLL ms.
chip16_t *core = decode_data.cpu;
chip16_write_memory16 (core, SND_MEM_ADDR, SND_MEM_ADDR, 0x0002);
chip16_write_memory16 (core, SND_MEM_ADDR, SND_MEM_ADDR,   1000);
chip16_write_memory16 (core, SND_MEM_ADDR, SND_MEM_ADDR,   (hh<<8)+ll);
endinstruction

instruction: SND3
pattern: opcode == 0x0c
mnemonic: "snd3 0x%x%x", hh, ll
// snd3 (command word: 0, 2) - play 1500Hz tone for HHLL ms.
chip16_t *core = decode_data.cpu;
chip16_write_memory16 (core, SND_MEM_ADDR, SND_MEM_ADDR, 0x0002);
chip16_write_memory16 (core, SND_MEM_ADDR, SND_MEM_ADDR,   1500);
chip16_write_memory16 (core, SND_MEM_ADDR, SND_MEM_ADDR,   (hh<<8)+ll);
endinstruction

instruction: ADDI
pattern: opcode == 0x40
mnemonic: "addi r%d, 0x%x", x, ((hh << 8) + ll)

uint16 tmp = decode_data.cpu->chip16_reg[x];
uint32 res = decode_data.cpu->chip16_reg[x] + (hh << 8) + ll;

decode_data.cpu->chip16_reg[x] += ((hh << 8) + ll);
if (BIT_16(res) == 1)
    SET_CARRY(decode_data.cpu->flags);
else
    CLR_CARRY(decode_data.cpu->flags);

if (decode_data.cpu->chip16_reg[x] == 0)
    SET_ZERO(decode_data.cpu->flags);
else
    CLR_ZERO(decode_data.cpu->flags);

if (((BIT_15(tmp) == 0) && (BIT_15((hh << 8) + ll) == 0) && (BIT_15(res) == 1)) ||
    ((BIT_15(tmp) == 1) && (BIT_15((hh << 8) + ll) == 1) && (BIT_15(res) == 0)))

    SET_OVRFLW(decode_data.cpu->flags);
else
    CLR_OVRFLW(decode_data.cpu->flags);

if (BIT_15(res) == 1)
    SET_NEG(decode_data.cpu->flags);
else
    CLR_NEG(decode_data.cpu->flags);

endinstruction

instruction: ADD_XYZ
pattern: opcode == 0x42
mnemonic: "add r%d, r%d, r%d", x, y, z

chip16_t* core = decode_data.cpu;
uint16 X = core->chip16_reg[x];
uint16 Y = core->chip16_reg[y];
core->chip16_reg[z] = X + Y;
uint32 Z = X + Y;

if (BIT_16(Z) == 1)
    SET_CARRY(core->flags);
else
    CLR_CARRY(core->flags);

if (BIT_15(core->chip16_reg[z]) == 1)
    SET_NEG(core->flags);
else
    CLR_NEG(core->flags);

if (core->chip16_reg[z] == 0) SET_ZERO(core->flags);
else        CLR_ZERO(core->flags);

if (((BIT_15(X) == 0) && (BIT_15(Y) == 0) && (BIT_15(Z) == 1)) ||
    ((BIT_15(X) == 1) && (BIT_15(Y) == 1) && (BIT_15(Z) == 0)))
    SET_OVRFLW(core->flags);
else
    CLR_OVRFLW(core->flags);

endinstruction

instruction: LDI_SP
pattern: opcode == 0x21
mnemonic: "ldi sp, 0x%x", ((hh << 8) + ll)

chip16_set_sp(decode_data.cpu, (uint64_t) ((hh << 8) + ll));

endinstruction

instruction: STM_XY
pattern: opcode == 0x31
mnemonic: "stm r%d, r%d", x, y

chip16_write_memory16(decode_data.cpu, decode_data.cpu->chip16_reg[y], 
        decode_data.cpu->chip16_reg[y], decode_data.cpu->chip16_reg[x]);

endinstruction

instruction: MULI_X
pattern: opcode == 0x90
mnemonic: "muli r%d, 0x%x", x, ((hh<<8)+ll)

chip16_t* core = decode_data.cpu;
uint32 res = core->chip16_reg[x] * ((hh<<8)+ll);
core->chip16_reg[x] *= ((hh<<8)+ll);

if (res > MAX_REG)
    SET_CARRY(core->flags);
else
    CLR_CARRY(core->flags);

if (core->chip16_reg[x] == 0)
    SET_ZERO(core->flags);
else
    CLR_ZERO(core->flags);

if (BIT_15(res) == 1)
    SET_NEG(core->flags);
else
    CLR_NEG(core->flags);

endinstruction

instruction: POPALL
pattern: opcode == 0xc3
mnemonic: "popall"

chip16_t* core = decode_data.cpu;
for (int i = 0; i < NUMB_OF_REGS; i++) {
    chip16_set_sp (core, chip16_get_sp (core) - 2);
    chip16_read_memory (core, chip16_get_sp (core), 2, (uint8*)(&(core->chip16_reg[i])), 1);
}

endinstruction

instruction: DIV
pattern: opcode == 0xa1
mnemonic: "div r%d, r%d", x, y

uint16 res;
chip16_t* core = decode_data.cpu;
if (core->chip16_reg[y] != 0) {
    if (core->chip16_reg[x] % core->chip16_reg[y] != 0) SET_CARRY(core->flags);
    else                                                CLR_CARRY(core->flags);
    core->chip16_reg[x] = core->chip16_reg[x] / core->chip16_reg[y];
    res = core->chip16_reg[x];

    if (res == 0) SET_ZERO(core->flags);
    else          CLR_ZERO(core->flags);

    if ((res & (1 << 15)) != 0) SET_NEG(core->flags);
    else                        CLR_NEG(core->flags);
    }
else SIM_LOG_INFO(1, core->obj, 0, "Dividing by zero!\n");

endinstruction

instruction: AND_XYZ
pattern: opcode == 0x62
mnemonic: "and r%d, r%d, r%d", x, y, z

chip16_t* core = decode_data.cpu;
uint16 res = (core->chip16_reg[x]) & (core->chip16_reg[y]);
core->chip16_reg[z] = res;
if (res == 0) {
    SET_ZERO(core->flags);
}
else {
    CLR_ZERO(core->flags);
    if (BIT_15(res) != 0) SET_NEG(core->flags);
    else                  CLR_NEG(core->flags);
}

endinstruction

instruction: NEGI
pattern: opcode == 0xe3
mnemonic: "negi r%d, 0x%x", x, ((hh<<8)+ll)

chip16_t* core = decode_data.cpu;
uint16 HHLL = (hh<<8)+ll;
if (HHLL == 0) {
    SET_ZERO(core->flags);
    core->chip16_reg[x] = HHLL;
    CLR_NEG(core->flags);
}
else {
    HHLL = ~HHLL + 1;
    core->chip16_reg[x] = HHLL;
    if (BIT_15(HHLL) != 0) {
        SET_NEG(core->flags);
    }
    else CLR_NEG(core->flags);
    CLR_ZERO(core->flags);
}

endinstruction

instruction: NEG_XY
pattern: opcode == 0xe5
mnemonic: "neg r%d, r%d", x, y

chip16_t* core = decode_data.cpu;
core->chip16_reg[x] = ~(core->chip16_reg[y]) + 1;
if (core->chip16_reg[x] != 0) {
    if (BIT_15(core->chip16_reg[x]) != 0) {
        SET_NEG(core->flags);
    }
    else {
        CLR_NEG(core->flags);
    }
    CLR_ZERO(core->flags);
}
else {
    SET_ZERO(core->flags);
    CLR_NEG(core->flags);
}

endinstruction

instruction: DIV_XYZ
pattern: opcode == 0xa2
mnemonic: "div r%d, r%d, r%d", x, y, z

uint16 res;
chip16_t* core = decode_data.cpu;
if(core->chip16_reg[y] != 0) {
    if (core->chip16_reg[x] % core->chip16_reg[y] != 0) SET_CARRY(core->flags);
    else                                                CLR_CARRY(core->flags);
    res = core->chip16_reg[x] / core->chip16_reg[y];
    core->chip16_reg[z] = res;

    if (res == 0) SET_ZERO(core->flags);
    else          CLR_ZERO(core->flags);

    if ((res & (1 << 15)) != 0) SET_NEG(core->flags);
    else                        CLR_NEG(core->flags);
}
else SIM_LOG_INFO(1, core->obj, 0, "Dividing by zero!\n");

endinstruction

instruction: XOR
pattern: opcode == 0x81
mnemonic: "xor r%d, r%d", x, y

uint16 res;
chip16_t* core = decode_data.cpu;
core->chip16_reg[x] = res = core->chip16_reg[x] ^ core->chip16_reg[y];

if (res == 0) SET_ZERO(core->flags);
else          CLR_ZERO(core->flags);

if ((res & (1 << 15)) != 0) SET_NEG(core->flags);
else                        CLR_NEG(core->flags);

endinstruction

instruction: REMI
pattern: opcode == 0xa6
mnemonic: "remi r%d, 0x%x", x, (hh<<8)+ll

chip16_t* core = decode_data.cpu;
uint16 res;
core->chip16_reg[x] = res = core->chip16_reg[x] % ((hh<<8)+ll);

if (res == 0) SET_ZERO(core->flags);
else          CLR_ZERO(core->flags);

if ((res & (1 << 15)) != 0) SET_NEG(core->flags);
else                        CLR_NEG(core->flags);

endinstruction

instruction: NOTI
pattern: opcode == 0xe0
mnemonic: "noti r%d, 0x%x", x, (hh<<8)+ll

uint16 res;
chip16_t* core = decode_data.cpu;
core->chip16_reg[x] = res = ~((hh<<8)+ll) & 0xFFFF;

if (res == 0) SET_ZERO(core->flags);
else          CLR_ZERO(core->flags);

if ((res & (1 << 15)) != 0) SET_NEG(core->flags);
else                        CLR_NEG(core->flags);

endinstruction

instruction: CALL_HHLL
pattern: opcode == 0x14
mnemonic: "call 0x%x", (hh<<8)+ll
attributes: branch

chip16_t* core = decode_data.cpu;
chip16_write_memory16(
    core,
    chip16_get_sp(core),
    chip16_get_sp(core),
    chip16_get_pc(core));
chip16_set_sp(core, chip16_get_sp(core) + 2);
chip16_set_pc(core, (hh<<8)+ll);
endinstruction

instruction: RET
pattern: opcode == 0x15
mnemonic: "ret"
attributes: branch

chip16_t* core = decode_data.cpu;
chip16_set_sp(core, chip16_get_sp(core) - 2);
uint16 tmp = chip16_read_memory16(core,
        chip16_get_sp(core),
        chip16_get_sp(core));
chip16_set_pc(core, tmp);
endinstruction

instruction: JMP_X
pattern: opcode == 0x16
mnemonic: "jmp r%d", x
attributes: branch

chip16_set_pc(decode_data.cpu, decode_data.cpu->chip16_reg[x]);
endinstruction

instruction: Cx_HHLL
pattern: opcode == 0x17
mnemonic: "C %d 0x%x", x, (hh<<8)+ll
attributes: branch

chip16_t* core = decode_data.cpu;

// TODO: doesn't work because not according specification
// check_conditional_code func not implemeted

if (chip16_check_conditional_code(core, x)){
    chip16_write_memory16(
        core,
        chip16_get_sp(core),
        chip16_get_sp(core),
        chip16_get_pc(core));
    chip16_set_sp(core, chip16_get_sp(core) + 2);
    chip16_set_pc(core, (hh<<8)+ll);
}
endinstruction

instruction: CALL_X
pattern: opcode == 0x18
mnemonic: "call r%d", x
attributes: branch

chip16_t* core = decode_data.cpu;
chip16_write_memory16(
    core,
    chip16_get_sp(core),
    chip16_get_sp(core),
    chip16_get_pc(core));
chip16_set_sp(core, chip16_get_sp(core) + 2);
chip16_set_pc(core, core->chip16_reg[x]);
endinstruction

instruction: LDI_X
pattern: opcode == 0x20
mnemonic: "ldi r%d, 0x%x", x, (hh<<8)+ll

decode_data.cpu->chip16_reg[x] = (hh<<8)+ll;

endinstruction 

instruction: STM_X
pattern: opcode == 0x30
mnemonic: "stm r%d, 0x%x", x, (hh<<8)+ll

chip16_t* core = decode_data.cpu;
chip16_write_memory16(
    core,
    (hh<<8)+ll,
    (hh<<8)+ll,
    core->chip16_reg[x]);

endinstruction

instruction: POP
pattern: opcode == 0xc1
mnemonic: "pop r%d", x

chip16_t* core = decode_data.cpu;
chip16_set_sp(core, chip16_get_sp(core) - 2);
chip16_read_memory(
    core,
    chip16_get_sp(core), 2,
    (uint8*)(&(core->chip16_reg[x])), 1);

endinstruction

instruction: CMPI_X
pattern: opcode == 0x53
mnemonic: "cmpi r%d, 0x%x%x", x, hh, ll

chip16_t* core = decode_data.cpu;
uint16 X = core->chip16_reg[x];
uint16 Y = (hh<<8) | ll;
uint32 Z = X - Y;

if (BIT_16(Z) == 1)
    SET_CARRY(core->flags);
else
    CLR_CARRY(core->flags);

if (BIT_15(Z) == 1)
    SET_NEG(core->flags);
else
    CLR_NEG(core->flags);

if (X == Y) SET_ZERO(core->flags);
else        CLR_ZERO(core->flags);

if (((BIT_15(X) == 1) && (BIT_15(Y) == 0) && (BIT_15(Z) == 0)) ||
    ((BIT_15(X) == 0) && (BIT_15(Y) == 1) && (BIT_15(Z) == 1)))
    SET_OVRFLW(core->flags);
else
    CLR_OVRFLW(core->flags);

endinstruction
