class chip8{
 public:
    chip8();
    ~chip8();

    bool drawFlag;

    void emulateCycle();
    void debugRender();
    bool loadApplication(const char * filename);

    unsigned char gfx[64 * 32];
    unsigned char key[16];

 private:
    unsigned short opcode;
    unsigned short pc;
    unsigned short sp;
    unsigned short I;

    unsigned char V[16];
    unsigned short stack[16];
    unsigned char memory[4096];

    unsigned char delay_timer;
    unsigned char sound_timer;

    void init();
};
