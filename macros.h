typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

struct processor {
    u8 flag;    // flag
    u16 pc;     // program counter
    u8 sp;      // stack pointer
    u8 X;
    u8 Y;
    u8 A;
};

struct wires {
    u8 IRQ;     // Interrupt request: output dal VIC, input nel 6510
    u8 NMI;     // Interrupt non ignorabile
    u8 BA_RDY;  // la linea BA/RDY: in output dal VIC, dice alla CPU che può lavorare (alto) o no (basso)
    u8 LP;      // manipolazione di registri del VIC
    u8 phi;     // clock del VIC. esistono due phi ma sono alla stessa frequenza quindi we don't care
    u8 RW;      // 1 = lettura, 0 = scrittura utilizzando il bus
};

#define NEGATIVE_FLAG    0b10000000
#define OVERFLOW_FLAG    0b01000000
#define UNUSED_FLAG      0b00100000
#define BREAK_FLAG       0b00010000
#define DECIMAL_FLAG     0b00001000
#define INTERRUPT_FLAG   0b00000100
#define ZERO_FLAG        0b00000010
#define CARRY_FLAG       0b00000001

#define HEIGH       200
#define WIDTH       320

#define STACK       0x0100
#define KERNAL_ROM  0xE000
#define BASIC_ROM   0xA000
#define CHAR_ROM    0xD000

// 0X0001
#define LORAM       0b00000001
#define HIRAM       0b00000010
#define CHAREN      0b00000100

// MODALITA' DI INDIRIZZAMENTO
// costruzione dell'indirizzo nnnn
#define ABSOLUTE_ADDRESSING             (readAddr(++cpu.pc) | (readAddr(++cpu.pc) << 8))
// costruzione dell'indirizzo nnnn + X
#define ABSOLUTE_X_ADDRESSING           ((readAddr(++cpu.pc) | (readAddr(++cpu.pc) << 8)) + cpu.X)
// costruzione dell'indirizzo nnnn + Y
#define ABSOLUTE_Y_ADDRESSING           ((readAddr(++cpu.pc) | (readAddr(++cpu.pc) << 8)) + cpu.Y)
// costruzione di un indirizzo nn
#define ZERO_PAGE_ADDRESSING            readAddr(++cpu.pc)
// costruzione di un indirizzo nn, X
#define ZERO_PAGE_X_ADDRESSING          ((readAddr(++cpu.pc) + cpu.X) & 0x00ff)
// costruzione dell'indirizzo (nn, X)
#define INDEXED_INDIRECT_ADDRESSING     (readAddr((readAddr(++cpu.pc) + cpu.X) & 0x00ff) | (readAddr(((readAddr(cpu.pc) + cpu.X + 1) & 0x00ff)) << 8))
// costruzione dell'indirizzo (nn), Y
#define INDIRECT_INDEXED_ADDRESSING     ((readWord(readAddr(++cpu.pc)) + cpu.Y))

// COLORI
#define BLACK       0x0
#define WHITE       0x1
#define RED         0x2
#define CYAN        0x3
#define PINK        0x4
#define GREEN       0x5
#define BLUE        0x6
#define YELLOW      0x7
#define ORANGE      0x8
#define BROWN       0x9
#define LIGHT_RED   0xA
#define DARK_GRAY   0xB
#define MEDIUM_GRAY 0xC
#define LIGHT_GREEN 0xD
#define LIGHT_BLUE  0xE
#define LIGHT_GRAY  0xF

//  SLD
#define CELL_SIDE       3
#define REFRESH_RATE    (float)1/60