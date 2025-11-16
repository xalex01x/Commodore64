#include "funLib.h"

void x00() { error = 1; }

void x01()
{ // ORA (nn, X)
    cpu.A |= readAddr(INDEXED_INDIRECT_ADDRESSING);
    ZNflags(cpu.A);
    remainingCycles += 6;
}

void x05()
{ // ORA nn
    cpu.A |= readAddr(ZERO_PAGE_ADDRESSING);
    ZNflags(cpu.A);
    remainingCycles += 3;
}

void x06() { error = 1; }

void x08() { error = 1; }

void x09()
{ // ORA #nn
    cpu.A |= readAddr(++cpu.pc);
    ZNflags(cpu.A);
    remainingCycles += 2;
}

void x0A() { error = 1; }

void x0D()
{ // ORA nnnn
    cpu.A |= readAddr(ABSOLUTE_ADDRESSING);
    ZNflags(cpu.A);
    remainingCycles += 4;
}

void x0E() { error = 1; }

void x10()
{ // BPL
    if (cpu.flag & NEGATIVE_FLAG)
    {
        cpu.pc++;
        remainingCycles += 2;
    }
    else
    {
        u16 oldPc = cpu.pc;
        u16 offset = (readAddr(cpu.pc + 1) & NEGATIVE_FLAG) ? 0xff00 | readAddr(cpu.pc + 1) : 0x0000 | readAddr(cpu.pc + 1);
        cpu.pc = oldPc + offset + 1;
        remainingCycles += 3;
        if (((cpu.pc + 1) & 0xff00) != (oldPc & 0xff00))
            remainingCycles += 1;
    }
}

void x11() { error = 1; }

void x15() { error = 1; }

void x16() { error = 1; }

void x18()
{ // CLC
    cpu.flag &= ~CARRY_FLAG;
    remainingCycles += 2;
}

void x19() { error = 1; }

void x1A() { error = 1; }

void x1D() { error = 1; }

void x1E() { error = 1; }

void x20()
{ // JSR nnnn
    cpu.pc += 2;
    pushPC();
    cpu.pc = (readAddr(cpu.pc) << 8) | (readAddr(cpu.pc - 1) & 0x00ff) - 1;
    remainingCycles += 6;
}

void x21() { error = 1; }

void x24() { error = 1; }

void x25() { error = 1; }

void x26() { error = 1; }

void x28() { error = 1; }

void x29()
{ // AND #nn
    cpu.A &= readAddr(++cpu.pc);
    ZNflags(cpu.A);
    remainingCycles += 2;
}

void x2A()
{ // ROL A
    u8 oldCARRY_FLAG = cpu.flag & CARRY_FLAG;
    cpu.flag = (cpu.A & 0x80) ? cpu.flag | CARRY_FLAG : cpu.flag & ~CARRY_FLAG;
    cpu.A <<= 1;
    cpu.A |= oldCARRY_FLAG;
    ZNflags(cpu.A);
    remainingCycles += 2;
}

void x2C() { error = 1; }

void x2D()
{ // AND nnnn
    cpu.A &= readAddr(ABSOLUTE_ADDRESSING);
    ZNflags(cpu.A);
    remainingCycles += 4;
}

void x2E() { error = 1; }

void x30()
{ // BMI
    if (cpu.flag & NEGATIVE_FLAG)
    {
        u16 oldPc = cpu.pc;
        u16 offset = (readAddr(cpu.pc + 1) & NEGATIVE_FLAG) ? 0xff00 | readAddr(cpu.pc + 1) : 0x0000 | readAddr(cpu.pc + 1);
        cpu.pc = oldPc + offset + 1;
        remainingCycles += 3;
        if (((cpu.pc + 1) & 0xff00) != (oldPc & 0xff00))
            remainingCycles += 1;
    }
    else
    {
        cpu.pc++;
        remainingCycles += 2;
    }
}

void x31() { error = 1; }

void x35() { error = 1; }

void x36() { error = 1; }

void x38() { error = 1; }

void x39() { error = 1; }

void x3A() { error = 1; }

void x3D() { error = 1; }

void x3E() { error = 1; }

void x40()
{ // RTI
    returnInterrupt();
}

void x41() { error = 1; }

void x45() { error = 1; }

void x46() { error = 1; }

void x48()
{ // PHA
    push(cpu.A);
    remainingCycles += 3;
}

void x49() { error = 1; }

void x4A() { error = 1; }

void x4C()
{ // JMP nnnn
    cpu.pc = (ABSOLUTE_ADDRESSING)-1;
    remainingCycles += 3;
}

void x4D() { error = 1; }

void x4E() { error = 1; }

void x50() { error = 1; }

void x51() { error = 1; }

void x55() { error = 1; }

void x56() { error = 1; }

void x58() { error = 1; }

void x59() { error = 1; }

void x5A() { error = 1; }

void x5D() { error = 1; }

void x5E() { error = 1; }

void x60()
{ // RTS
    popPC();
    remainingCycles += 6;
}

void x61() { error = 1; }

void x65() { error = 1; }

void x66() { error = 1; }

void x68()
{ // PLA
    cpu.A = pop();
    ZNflags(cpu.A);
    remainingCycles += 4;
}

void x69()
{ // ADC #nn
    if (cpu.flag & DECIMAL_FLAG)
    {
        u8 value = readAddr(++cpu.pc);
        u8 al = (cpu.A & 0x0F) + (value & 0x0F) + (cpu.flag & CARRY_FLAG);
        u8 ah = (cpu.A >> 4) + (value >> 4);
        if (al > 9)
        {
            al -= 10;
            ah++;
        }
        if (ah > 9)
        {
            ah -= 10;
            cpu.flag |= CARRY_FLAG;
        }
        else
        {
            cpu.flag &= ~CARRY_FLAG;
        }
        cpu.A = (ah << 4) | (al & 0x0F);
        ZNflags(cpu.A);
    }
    else
    {
        u16 res = cpu.A + readAddr(++cpu.pc) + (cpu.flag & CARRY_FLAG);
        cpu.A = res;
        ZNflags(cpu.A);
        cpu.flag = (res >> 8) | (cpu.flag & ~CARRY_FLAG);
        remainingCycles += 2;
    }
}

void x6A() { error = 1; }

void x6C()
{ // JMP (nnnn)
    u16 addr = ABSOLUTE_ADDRESSING;
    if ((addr & 0x00ff) == 0x00ff)
    {
        cpu.pc = (readAddr(addr) | (readAddr(addr & 0x00ff) << 8)) - 1;
    }
    else
    {
        cpu.pc = readWord(addr) - 1;
    }
    remainingCycles += 5;
}

void x6D() { error = 1; }

void x6E() { error = 1; }

void x70() { error = 1; }

void x71() { error = 1; }

void x75() { error = 1; }

void x76() { error = 1; }

void x78()
{ // SEI
    cpu.flag |= INTERRUPT_FLAG;
    remainingCycles += 2;
}

void x79() { error = 1; }

void x7A() { error = 1; }

void x7D() { error = 1; }

void x7E() { error = 1; }

void x81() { error = 1; }

void x84()
{ // STY nn
    writeAddr(ZERO_PAGE_ADDRESSING, cpu.Y);
    remainingCycles += 3;
}

void x85()
{ // STA nn
    writeAddr(ZERO_PAGE_ADDRESSING, cpu.A);
    remainingCycles += 3;
}

void x86()
{ // STX nn
    writeAddr(ZERO_PAGE_ADDRESSING, cpu.X);
    remainingCycles += 3;
}

void x88()
{ // DEY
    cpu.Y--;
    ZNflags(cpu.Y);
    remainingCycles += 2;
}

void x89() { error = 1; }

void x8A()
{ // TXA
    cpu.A = cpu.X;
    ZNflags(cpu.A);
    remainingCycles += 2;
}

void x8C()
{ // STY nnnn
    writeAddr(ABSOLUTE_ADDRESSING, cpu.Y);
    remainingCycles += 4;
}

void x8D()
{ // STA nnnn
    writeAddr(ABSOLUTE_ADDRESSING, cpu.A);
    remainingCycles += 4;
}

void x8E()
{ // STX nnnn
    writeAddr(ABSOLUTE_ADDRESSING, cpu.X);
    remainingCycles += 4;
}

void x90()
{ // BCC
    if (cpu.flag & CARRY_FLAG)
    {
        cpu.pc++;
        remainingCycles += 2;
    }
    else
    {
        u16 oldPc = cpu.pc;
        u16 offset = (readAddr(cpu.pc + 1) & NEGATIVE_FLAG) ? 0xff00 | readAddr(cpu.pc + 1) : 0x0000 | readAddr(cpu.pc + 1);
        cpu.pc = oldPc + offset + 1;
        remainingCycles += 3;
        if (((cpu.pc + 1) & 0xff00) != (oldPc & 0xff00))
            remainingCycles += 1;
    }
}

void x91()
{ // STA (nn), Y
    writeAddr(INDIRECT_INDEXED_ADDRESSING, cpu.A);
    remainingCycles += 6;
}

void x94()
{ // STY nn, X
    writeAddr(ZERO_PAGE_X_ADDRESSING, cpu.Y);
    remainingCycles += 4;
}

void x95()
{ // STA nn, X
    writeAddr(ZERO_PAGE_X_ADDRESSING, cpu.A);
    remainingCycles += 4;
}

void x96() { error = 1; }

void x98()
{ // TYA
    cpu.A = cpu.Y;
    ZNflags(cpu.A);
    remainingCycles += 2;
}

void x99()
{ // STA nnnn, Y
    writeAddr(ABSOLUTE_Y_ADDRESSING, cpu.A);
    remainingCycles += 5;
}

void x9A()
{ // TXS
    cpu.sp = cpu.X;
    remainingCycles += 2;
}

void x9E() { error = 1; }

void x9D()
{ // STA nnnn, X
    writeAddr(ABSOLUTE_X_ADDRESSING, cpu.A);
    remainingCycles += 5;
}

void xA0()
{ // LDY #nn
    cpu.Y = readAddr(++cpu.pc);
    ZNflags(cpu.Y);
    remainingCycles += 2;
}

void xA1() { error = 1; }

void xA2()
{ // LDX #nn
    cpu.X = readAddr(++cpu.pc);
    ZNflags(cpu.X);
    remainingCycles += 2;
}

void xA4()
{ // LDY nn
    cpu.Y = readAddr(ZERO_PAGE_ADDRESSING);
    ZNflags(cpu.Y);
    remainingCycles += 3;
}

void xA5()
{ // LDA nn
    cpu.A = readAddr(ZERO_PAGE_ADDRESSING);
    ZNflags(cpu.A);
    remainingCycles += 3;
}

void xA6()
{ // LDX nn
    cpu.X = readAddr(ZERO_PAGE_ADDRESSING);
    ZNflags(cpu.X);
    remainingCycles += 3;
}

void xA8()
{ // TAY
    cpu.Y = cpu.A;
    ZNflags(cpu.Y);
    remainingCycles += 2;
}

void xA9()
{ // LDA #nn
    cpu.A = readAddr(++cpu.pc);
    ZNflags(cpu.A);
    remainingCycles += 2;
}

void xAA()
{ // TAX
    cpu.X = cpu.A;
    ZNflags(cpu.X);
    remainingCycles += 2;
}

void xAC()
{ // LDY nnnn
    cpu.Y = readAddr(ABSOLUTE_ADDRESSING);
    ZNflags(cpu.Y);
    remainingCycles += 4;
}

void xAD()
{ // LDA nnnn
    cpu.A = readAddr(ABSOLUTE_ADDRESSING);
    ZNflags(cpu.A);
    remainingCycles += 4;
}

void xAE() { error = 1; }

void xB0()
{ // BCS
    if (cpu.flag & CARRY_FLAG)
    {
        u16 oldPc = cpu.pc;
        u16 offset = (readAddr(cpu.pc + 1) & NEGATIVE_FLAG) ? 0xff00 | readAddr(cpu.pc + 1) : 0x0000 | readAddr(cpu.pc + 1);
        cpu.pc = oldPc + offset + 1;
        remainingCycles += 3;
        if (((cpu.pc + 1) & 0xff00) != (oldPc & 0xff00))
            remainingCycles += 1;
    }
    else
    {
        cpu.pc++;
        remainingCycles += 2;
    }
}

void xB1()
{ // LDA (nn), Y
    u16 addr = INDIRECT_INDEXED_ADDRESSING;
    cpu.A = readAddr(addr);
    ZNflags(cpu.A);
    remainingCycles += 5;
    if ((addr & 0xff00) != ((addr - cpu.Y) & 0xff00))
        remainingCycles += 1;
}

void xB4()
{ // LDY nn, X
    cpu.Y = readAddr(ZERO_PAGE_X_ADDRESSING);
    ZNflags(cpu.Y);
    remainingCycles += 4;
}

void xB5()
{ // LDA nn, X
    cpu.A = readAddr(ZERO_PAGE_X_ADDRESSING);
    ZNflags(cpu.A);
    remainingCycles += 4;
}

void xB6() { error = 1; }

void xB8() { error = 1; }

void xB9()
{ // LDA nnnn, Y
    u16 addr = ABSOLUTE_Y_ADDRESSING;
    cpu.A = readAddr(addr);
    ZNflags(cpu.A);
    remainingCycles += 4;
    if ((addr & 0xff00) != ((addr - cpu.Y) & 0xff00))
        remainingCycles += 1;
}

void xBA()
{ // TSX
    cpu.X = cpu.sp;
    ZNflags(cpu.X);
    remainingCycles += 2;
}

void xBC() { error = 1; }

void xBD()
{ // LDA nnnn, X
    u16 base = readWord(cpu.pc + 1);
    u16 addr = ABSOLUTE_X_ADDRESSING;
    cpu.A = readAddr(addr);
    ZNflags(cpu.A);
    remainingCycles += 4;
    if ((addr & 0xff00) != (base & 0xff00))
        remainingCycles += 1;
}

void xBE() { error = 1; }

void xC0() { error = 1; }

void xC1() { error = 1; }

void xC4() { error = 1; }

void xC5() { error = 1; }

void xC6() { error = 1; }

void xC8()
{ // INY
    cpu.Y++;
    ZNflags(cpu.Y);
    remainingCycles += 2;
}

void xC9() { error = 1; }

void xCA()
{ // DEX
    cpu.X--;
    ZNflags(cpu.X);
    remainingCycles += 2;
}

void xCC() { error = 1; }

void xCD() { error = 1; }

void xCE() { error = 1; }

void xD0()
{ // BNE
    if (cpu.flag & ZERO_FLAG)
    {
        cpu.pc++;
        remainingCycles += 2;
    }
    else
    {
        u16 oldPc = cpu.pc;
        u16 offset = (readAddr(cpu.pc + 1) & NEGATIVE_FLAG) ? 0xff00 | readAddr(cpu.pc + 1) : 0x0000 | readAddr(cpu.pc + 1);
        cpu.pc = oldPc + offset + 1;
        remainingCycles += 3;
        if (((cpu.pc + 1) & 0xff00) != (oldPc & 0xff00))
            remainingCycles += 1;
    }
}

void xD1()
{ // CMP (nn), Y
    u16 addr = INDIRECT_INDEXED_ADDRESSING;
    u8 res = cpu.A - readAddr(addr);
    ZNflags(res);
    cpu.flag = cpu.A >= readAddr(addr) ? cpu.flag & ~CARRY_FLAG : cpu.flag | CARRY_FLAG;
    remainingCycles += 5;
    if ((addr & 0xff00) != ((addr - cpu.Y) & 0xff00))
        remainingCycles += 1;
}

void xD5() { error = 1; }

void xD6() { error = 1; }

void xD8()
{ // CLD
    cpu.flag &= (~DECIMAL_FLAG);
    remainingCycles += 2;
}

void xD9() { error = 1; }

void xDA() { error = 1; }

void xDD()
{ // CMP nnnn, X
    u16 base = readWord(cpu.pc + 1);
    u16 addr = ABSOLUTE_X_ADDRESSING;
    u8 res = cpu.A - readAddr(addr);
    ZNflags(res);
    cpu.flag = cpu.A >= readAddr(addr - cpu.X) ? cpu.flag & ~CARRY_FLAG : cpu.flag | CARRY_FLAG;
    remainingCycles += 4;
    if ((addr & 0xff00) != (base & 0xff00))
        remainingCycles += 1;
}

void xDE() { error = 1; }

void xE0()
{ // CPX #nn
    u8 res = cpu.X - readAddr(++cpu.pc);
    ZNflags(res);
    cpu.flag = cpu.X >= readAddr(cpu.pc) ? cpu.flag & ~CARRY_FLAG : cpu.flag | CARRY_FLAG;
    remainingCycles += 2;
}

void xE1() { error = 1; }

void xE5() { error = 1; }

void xE6()
{ // INC nn
    u16 addr = ZERO_PAGE_ADDRESSING;
    u8 value = ram[addr];
    writeAddr(addr, value + 1);
    ZNflags(ram[addr]);
    remainingCycles += 5;
}

void xE8()
{ // INX
    cpu.X++;
    ZNflags(cpu.X);
    remainingCycles += 2;
}

void xE9() { error = 1; }

void xEA() { error = 1; }

void xEC() { error = 1; }

void xED() { error = 1; }

void xEE() { error = 1; }

void xF0()
{ // BEQ
    if (cpu.flag & ZERO_FLAG)
    {
        u16 oldPc = cpu.pc;
        u16 offset = (readAddr(cpu.pc + 1) & NEGATIVE_FLAG) ? 0xff00 | readAddr(cpu.pc + 1) : 0x0000 | readAddr(cpu.pc + 1);
        cpu.pc = oldPc + offset + 1;
        remainingCycles += 3;
        if (((cpu.pc + 1) & 0xff00) != (oldPc & 0xff00))
            remainingCycles += 1;
    }
    else
    {
        cpu.pc++;
        remainingCycles += 2;
    }
}

void xF1() { error = 1; }

void xF5() { error = 1; }

void xF6() { error = 1; }

void xF8() { error = 1; }

void xF9() { error = 1; }

void xFA() { error = 1; }

void xFD() { error = 1; }

void xFE() { error = 1; }