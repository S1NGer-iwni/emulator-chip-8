#define TRUE 1
#define FALSE 0
#define MAXSTACK 16

void emulateCycle(void);
void debugRender(void);
int loadApplication(const char *filename);

void init(void);

/*******************************
	void push(unsigned short i);
	unsigned short pop(void);

*******************************/

typedef struct chip8 {

	unsigned short pc;
	unsigned short sp;
	unsigned short opcode;
	unsigned short I;

	unsigned char memory[4096];
	unsigned char V[16];
	unsigned short stack[MAXSTACK];

	unsigned char gfx[64 * 32];
	unsigned char key[16];

	unsigned char delay_timer;
	unsigned char sound_timer;

	unsigned short drawFlag;
	unsigned short tos;

};
