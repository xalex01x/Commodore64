#include <iostream>
#include <SDL2/SDL.h>
#include <time.h>
#include <fstream>
#include <cstring>
#include "instructionSet.h"

using namespace std;

void pch() // PlaCeHolder per le funzioni inesistenti
{
    error = 1;
}

void (*instructionSet[256])() = {
    x00, x01, pch, pch, pch, x05, x06, pch, x08, x09, x0A, pch, pch, x0D, x0E, pch,
    x10, x11, pch, pch, pch, x15, x16, pch, x18, x19, pch, pch, pch, x1D, x1E, pch,
    x20, x21, pch, pch, x24, x25, x26, pch, x28, x29, x2A, pch, x2C, x2D, x2E, pch,
    x30, x31, pch, pch, pch, x35, x36, pch, x38, x39, pch, pch, pch, x3D, x3E, pch,
    x40, x41, pch, pch, pch, x45, x46, pch, x48, x49, x4A, pch, x4C, x4D, x4E, pch,
    x50, x51, pch, pch, pch, x55, x56, pch, x58, x59, pch, pch, pch, x5D, x5E, pch,
    x60, x61, pch, pch, pch, x65, x66, pch, x68, x69, x6A, pch, x6C, x6D, x6E, pch,
    x70, x71, pch, pch, pch, x75, x76, pch, x78, x79, pch, pch, pch, x7D, x7E, pch,
    pch, x81, pch, pch, x84, x85, x86, pch, x88, pch, x8A, pch, x8C, x8D, x8E, pch,
    x90, x91, pch, pch, x94, x95, x96, pch, x98, x99, x9A, pch, pch, x9D, pch, pch,
    xA0, xA1, xA2, pch, xA4, xA5, xA6, pch, xA8, xA9, xAA, pch, xAC, xAD, xAE, pch,
    xB0, xB1, pch, pch, xB4, xB5, xB6, pch, xB8, xB9, xBA, pch, xBC, xBD, xBE, pch,
    xC0, xC1, pch, pch, xC4, xC5, xC6, pch, xC8, xC9, xCA, pch, xCC, xCD, xCE, pch,
    xD0, xD1, pch, pch, pch, xD5, xD6, pch, xD8, xD9, pch, pch, pch, xDD, xDE, pch,
    xE0, xE1, pch, pch, pch, xE5, xE6, pch, xE8, xE9, xEA, pch, xEC, xED, xEE, pch,
    xF0, xF1, pch, pch, pch, xF5, xF6, pch, xF8, xF9, pch, pch, pch, xFD, xFE, pch};

processor cpu;
wires bus;
u8 ram[16 * 0x1000];      // RAM: 16 pagine da 0x1000 byte ciscuna
u8 rom[5 * 0x1000];       // ROM: 5 pagine da 0x1000 byte ciscuna
u8 IOreg[0xFFF];          // registri IO
u8 screen[HEIGH * WIDTH]; // 320 per 200 pixel

u8 opcode;
u16 remainingCycles = 0;
u16 clockNUmber = 0;

bool error = false;
bool debug = false; // per la funzione sottostante
bool addrOpcodeInfo = false;
bool fetchDebug = false;

// 0xfce2,        0xfd02,      0xfcef,          0xfda3,     0xff6e,       0xfcf5,     0xfd15
//"Entry point", "Check ROM", "ROM stat code", "I/O init", "CIA timers", "RAM test", "Vector set"

bool taken = false; // per non ciclare sulle interrupt      DA TOGLIERE PRIMA O POI

/*
    L'emulatore crede di essere NTSC
    0xd11 contiene: 10011011
    0xd16 contiene: 00001000
*/

int cpuCycle()
{
    if (fetchDebug)
    {
        cout << " A:";
        printbyte(cpu.A);
        cout << "\t X:";
        printbyte(cpu.X);
        cout << "\t Y:";
        printbyte(cpu.Y);
        cout << "\t F:";
        printbyte(cpu.flag);
        cout << "\t S:";
        printbyte(cpu.sp);
    }

    opcode = fetch(); // FETCH

    if (addrOpcodeInfo)
    {
        cout << "addr: " << hex << cpu.pc << " op: ";
        printbyte(opcode);
        cout << endl;
    }

    instructionSet[opcode](); // chiamiamo la rispettiva funzione

    if (error == 0)
        return 0;
    cout << endl
         << "indirizzo " << hex << cpu.pc << ": opcode non riconosciuto: ";
    printbyte(opcode);
    cout << endl;
    return 1;
}

void vicCycle()
{
}

void getInput()
{
}

int iteration()
{
    // INVIO DI COMANDI AL VIC
    vicCycle();

    // GESTIONE CPU
    if (remainingCycles)
    {
        clockNUmber++;
        remainingCycles--;
        return 0;
    }

    if (!bus.BA_RDY)
    {
        return cpuCycle();
    }

    // CONTROLLO INTERRUPT
    if ((!bus.IRQ && !(cpu.flag & INTERRUPT)))
    {
        cout << "Gestione interrupt IRQ" << endl;
        interrupt('i');
        taken = 1;
    }
    if (!bus.NMI)
    {
        interrupt('n');
        taken = 1;
    }

    getInput();
    return 0;
}

void init()
{
    // INIZIALIZZAZIONE ROM
    char path[64];
    strcpy(path, "./basic.901226-01.bin");
    ROMRead(path, 0);
    strcpy(path, "./characters.901225-01.bin");
    ROMRead(path, 0x1000 * 2);
    strcpy(path, "./kernal.901227-03.bin");
    ROMRead(path, 0x1000 * 3);

    // INIZIALIZZAZIONE RAM
    ram[0x00] = 0x2F;
    ram[0x01] = 0x37;

    // INIZIALIZZAZIONE BUS
    bus.IRQ = 1;
    bus.NMI = 1;

    // INIZIALIZZAZIONE CPU
    cpu.flag = 0x00;
    cpu.pc = ((readAddr(0xFFFD) << 8) | readAddr(0xFFFC)) - 1; // 0xFFFC reset address

    // TUTTE LE OPERAZIONI INIZIALI SONO ESEGUITE ISTANTANEAMENTE ANZICHE' SEGUIRE IL NORMALE REFRESH RATE
    while (cpu.pc != 0xfd15)
        iteration();
}

SDL_Renderer *renderer;
SDL_Rect rect;

/*
void printDisplay(){
    for(int i = 0; i < HEIGH; i++){
        for(int j = 0; j < WIDTH; j++){
            if(display[i*WIDTH + j] == oldFrame[i*WIDTH + j]) continue;
            oldFrame[i*WIDTH + j] = display[i*WIDTH + j];
            if(display[i*WIDTH + j] == black) SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            else SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            rect.x = j*CELL_SIDE;
            rect.y = i*CELL_SIDE;
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}
*/

int main(int argc, char *argv[])
{
    /*
    char * rom;
    if(argc < 2) {
        std::cout<< "Specifica il percorso di una cartuccia." << std:: endl;
        return 0;
    } else rom = argv[1];
    */
    rect.w = CELL_SIDE;
    rect.h = CELL_SIDE;
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::cout << "Errore nell'inizializzare SDL: " << SDL_GetError() << std::endl;
        return 1;
    }
    SDL_Window *window = SDL_CreateWindow("C64",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          WIDTH * CELL_SIDE, HEIGH * CELL_SIDE,
                                          SDL_WINDOW_SHOWN);
    if (!window)
    {
        std::cout << "Errore nella creazione della finestra: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        std::cout << "Errore nella creazione del renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    bool isRunning = true;
    SDL_Event event;

    init();

    clock_t frameStart;
    float frameTime;

    while (isRunning)
    {
        frameStart = clock();
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                isRunning = false;
            }
        }
        if (iteration())
            isRunning = false;
        frameTime = (float)(clock() - frameStart) / CLOCKS_PER_SEC;
        SDL_RenderPresent(renderer);
        if (frameTime < REFRESH_RATE)
        {
            int delayTime = (int)((REFRESH_RATE - frameTime) * 1000);
            SDL_Delay(delayTime);
        }
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

/*
        if(opcode == 0x0E){ // ASL nnnn
            u16 addr = ABSOLUTE_ADDRESSING;
            u8 value = readAddr(addr);
            cpu.flag = (value >> 7) ? cpu.flag | CARRY : cpu.flag & ~CARRY;
            value <<= 1;
            ZNflags(value);
            writeAddr(addr, value);
            continue;
        }
        if(opcode == 0x28){ // PLP
            cpu.flag = pop();
            remainingCycles += 4;
            continue;
        }
        if(opcode == 0x38){ // SEC
            cpu.flag |= CARRY;
            remainingCycles += 2;
            continue;
        }
        if(opcode == 0x58){ // CLI
            cpu.flag &= ~INTERRUPT;
            remainingCycles += 2;
            continue;
        }

        if(opcode == 0x81){ // STA (nn, X)
            writeAddr(INDEXED_INDIRECT_ADDRESSING, cpu.A);
            remainingCycles += 6;
            continue;
        }
        if(opcode == 0x85){ // STA nn
            writeAddr(ZERO_PAGE_ADDRESSING, cpu.A);
            remainingCycles += 3;
            continue;
        }

        if(opcode == 0x8D){ // STA nnnn
            writeAddr(ABSOLUTE_ADDRESSING, cpu.A);
            remainingCycles += 4;
            continue;
        }
        if(opcode == 0xAE){ // LDX nnnn
            cpu.X = readAddr(ABSOLUTE_ADDRESSING);
            cpu.flag = (cpu.X) ? cpu.flag & ~ZERO : cpu.flag | ZERO;
            cpu.flag = (cpu.X & NEGATIVE) | (cpu.flag & ~NEGATIVE);
            remainingCycles += 4;
            continue;
        }
        if(opcode == 0xC4){ //CPY nn
            u8 res = cpu.Y - readAddr(ZERO_PAGE_ADDRESSING);
            ZNflags(res);
            cpu.flag = cpu.Y >= readAddr(readAddr(cpu.pc)) ? cpu.flag & ~CARRY : cpu.flag | CARRY;
            remainingCycles += 3;
            continue;
        }
        if(opcode == 0xC5){ // CMP nn
            u8 res = cpu.A - readAddr(ZERO_PAGE_ADDRESSING);
            ZNflags(res);
            cpu.flag = cpu.A >= readAddr(readAddr(cpu.pc)) ? cpu.flag & ~CARRY : cpu.flag | CARRY;
            remainingCycles += 3;
            continue;
        }
        if(opcode == 0xCD){ // CMP nnnn
            u16 base = ABSOLUTE_ADDRESSING;
            u8 res = cpu.A - readAddr(base);
            ZNflags(res);
            cpu.flag = cpu.A >= readAddr(base) ? cpu.flag & ~CARRY : cpu.flag | CARRY;
            remainingCycles += 4;
            continue;
        }
        */

/*
Memory Map:
  0000-03FF OS RAM
  0400-07FF Screen Memory (40x25)
  0800-9FFF RAM
  A000-BFFF RAM/BASIC
  C000-CFFF RAM
  D000-DFFF RAM/CHARSET/IO
  E000-FFFF RAM/KERNAL
https://www.c64-wiki.com/wiki/Memory_Map
https://www.c64-wiki.com/wiki/Keyboard


DOC: https://c64os.com/post/6502instructions

ROM DISASSEMBLY https://www.pagetable.com/c64ref/c64disasm/#F5DD
MEMORY MAP https://www.pagetable.com/c64ref/c64mem/

VIC: https://www.cebix.net/VIC-Article.txt
*/