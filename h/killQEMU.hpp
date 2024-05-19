inline void killQEMU()
{
    __asm__(
        "li t0, 0x5555\n"   // Učitajte 32-bitnu vrednost 0x5555 u registar t0
        "li t1, 0x100000\n" // Učitajte vrednost adrese 0x100000 u registar t1
        "sw t0, (t1)\n"     // Upisujemo vrednost iz t0 (0x5555) na adresu (t1) (0x100000)
    );
}