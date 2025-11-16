#include "macros.h"
using namespace std;

extern processor cpu;
extern u16 remainingCycles;
extern u8 opcode;
extern u8 rom[];
extern u8 ram[];
extern u8 IOreg[];
extern bool debug;
extern bool addrOpcodeInfo;
extern wires bus;
extern bool error;

void printbyte(u8 arg);
void printbinary(u8 arg);
void ROMRead(char *path, u16 start);
u8 readAddr(u16 addr);
void writeAddr(u16 addr, u8 value);
u16 readWord(u16 addr);
void ZNflags(u8 value);
u8 fetch();
void push(u8 reg);
u8 pop();
void pushPC();
void popPC();
void interrupt(char type);
void returnInterrupt();
void ramDump(u16 base, u16 offset);

void printbyte(u8 arg)
{
    cout << hex << static_cast<u16>(arg);
}

void printbinary(u8 arg)
{
    int i = 0;
    while (1)
    {
        cout << ((arg & 0x80) >> 7);
        i++;
        if (i == 8)
            return;
        arg <<= 1;
    }
}

void ROMRead(char *path, u16 start)
{
    ifstream infile(path, ios::in | ios::binary);
    if (!infile.is_open())
    {
        cout << "Errore nell'apertura della ROM" << endl;
        return;
    }
    int i = 0;
    while (!infile.eof())
    {
        rom[start + i] = infile.get();
        i++;
    }
}

u8 readAddr(u16 addr)
{
    // normale lettura dalla RAM
    if ((addr < BASIC_ROM) || (addr >= 0xC000 && addr < CHAR_ROM))
        return ram[addr]; // lettura RAM

    // lettura RAM/KERNAL
    if (addr >= KERNAL_ROM)
    {
        if (ram[0x0001] & HIRAM)
            return rom[addr - 0xB000]; // lettura KERNAL ROM
        return ram[addr];              // lettura RAM
    }

    // lettura RAM/CHAR/IO
    if (addr >= CHAR_ROM)
    {
        if (ram[0x0001] & CHAREN)
            return IOreg[addr & 0xFFF]; // lettura IO
        if (ram[0x0001] & HIRAM)
            return ram[addr];                 // lettura RAM
        return rom[(addr & 0x0FFF) | 0x2000]; // lettura ROM CHAR
    }

    // lettura RAM/BASIC
    if (ram[0x0001] & LORAM)
        return rom[addr - 0xA000]; // lettura ROM BASIC
    return ram[addr];              // lettura RAM
}

void writeAddr(u16 addr, u8 value)
{
    // se si prova a scrivere nei registri IO
    if (addr < KERNAL_ROM && addr >= CHAR_ROM && (ram[0x0001] & CHAREN))
    {
        IOreg[addr & 0x0FFF] = value;
        return;
    }
    ram[addr] = value;
}

u16 readWord(u16 addr)
{ // legge una word posta all'indirizzo [addr + 1, addr]
    return readAddr(addr) | (readAddr(addr + 1) << 8);
}

void ZNflags(u8 value)
{
    cpu.flag = (value) ? cpu.flag & ~ZERO_FLAG : cpu.flag | ZERO_FLAG;
    cpu.flag = (value & NEGATIVE_FLAG) | (cpu.flag & ~NEGATIVE_FLAG);
}

// 0xfce2,        0xfd02,      0xfcef,          0xfda3,     0xff6e,       0xfcf5,     0xfd15
//"Entry point", "Check ROM", "ROM stat code", "I/O init", "CIA timers", "RAM test", "Vector set"
int iterString = 0;
const char *stringhe[] = {"Entry point", "Check ROM", "ROM stat code", "I/O init", "CIA timers", "RAM test", "Vector set"};
u16 indirizzi[] = {0xfce2, 0xfd02, 0xfcef, 0xfda3, 0xff6e, 0xfcf5, 0xfd15};

u8 fetch()
{
    cpu.pc++;
    char tmp;
    if (iterString < 7 && cpu.pc == indirizzi[iterString])
    {
        cout << stringhe[iterString++] << endl;
        // cin >> tmp;
        // if(iterString == 7) ramDump(0xDC00,0x2FF);
    }
    return readAddr(cpu.pc);
}

void ramDump(u16 base, u16 offset)
{
    if ((base < BASIC_ROM) || (base >= 0xC000 && base < CHAR_ROM))
        cout << "base in RAM" << endl;
    if (base >= KERNAL_ROM)
    {
        if (ram[0x0001] & HIRAM)
            cout << "base in KERNAL ROM" << endl;
        else
            cout << "base in RAM" << endl;
        goto funzioneDump;
    }

    // lettura RAM/CHAR/IO
    if (base >= CHAR_ROM)
    {
        if (ram[0x0001] & CHAREN)
            cout << "base in IO" << endl;
        else if (ram[0x0001] & HIRAM)
            cout << "base in RAM" << endl;
        else
            cout << "base in ROM CHAR" << endl;
        goto funzioneDump;
    }

    // lettura RAM/BASIC
    if (ram[0x0001] & LORAM)
        cout << "lettura ROM BASIC" << endl;
    else
        cout << "lettura RAM" << endl;

funzioneDump:
    u16 i = base + offset;
    if (i < base)
    {
        cout << "Wrap-around" << endl;
        return;
    }
    for (; i >= base; i--)
    {
        cout << hex << i << "\t|\t";
        printbinary(readAddr(i));
        cout << endl;
    }
}

void push(u8 reg)
{
    ram[STACK + cpu.sp--] = reg;
}

u8 pop()
{
    return ram[STACK + (++cpu.sp)];
}

void pushPC()
{
    push((cpu.pc >> 8));
    push((cpu.pc & 0x00ff));
}

void popPC()
{
    cpu.pc = pop() | (pop() << 8);
}

u8 readVIC(u16 addr)
{   // per le letture del VIC
    // costruzione dell'indirizzo usando i 14 bit di indirizzo ed i bit invertiti del CIA 2
    u16 res = (~((0xFFFF & IOreg[0x0D00]) << 14) & (addr & 0xB000));
    if (!(addr & 0x6FFF) || !(addr & 0xEFFF))
        return rom[0x2000 | (0x0FFF & addr)];
    return ram[addr];
}

void interrupt(char type)
{
    pushPC();
    push(cpu.flag);
    if (type == 'n')
    {
        cpu.pc = ((readAddr(0xFFFB) << 8) | readAddr(0xFFFA)) - 1; // routine NMII
        cpu.flag |= INTERRUPT_FLAG;
        bus.NMI = 1;
    }
    else
        cpu.pc = ((readAddr(0xFFFF) << 8) | readAddr(0xFFFE)) - 1; // routine IRQ/BRK
    bus.IRQ = 1;
}

void returnInterrupt()
{
    cpu.flag = pop();
    popPC();
}