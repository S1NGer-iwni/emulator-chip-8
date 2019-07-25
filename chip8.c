#define use_SRT_SECURE_NO_WARRNINGS
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "chip8.h"


struct chip8 chip_8;
    unsigned char chip8_fontset[80] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, //0
	0x20, 0x60, 0x20, 0x20, 0x70, //1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
	0x90, 0x90, 0xF0, 0x10, 0x10, //4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
	0xF0, 0x10, 0x20, 0x40, 0x40, //7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
	0xF0, 0x90, 0xF0, 0x90, 0x90, //A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
	0xF0, 0x80, 0x80, 0x80, 0xF0, //C
	0xE0, 0x90, 0x90, 0x90, 0xE0, //D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
	0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};


void init(void)
{

	chip_8.pc	  =	0x200;
	chip_8.sp     =		0;
	chip_8.opcode =		0;
	chip_8.I      =		0;

	//clear display
	for (int i = 0; i < 2048; ++i)
	{
		chip_8.gfx[i] = 0;
	}

	//clear stack
	for(int i = 0; i < 16; ++i)
	{
		chip_8.stack[i] = 0;
	}
	//clear key & register
	for (int i = 0; i < 16; ++i)
	{
		chip_8.key[i] = chip_8.V[i] = 0;
	}
	//clear memory
	for (int i = 0; i < 4096; ++i)
	{
		chip_8.memory[i] = 0;
	}
	//load fontset
	for (int i = 0; i < 80; ++i)
	{
		chip_8.memory[i] = chip8_fontset[i];
	}

	//init timer
	chip_8.delay_timer = 0;
	chip_8.sound_timer = 0;

	chip_8.drawFlag = TRUE;

	srand(time(NULL));
}

void emulateCycle(void)
{

	chip_8.opcode = chip_8.memory[chip_8.pc] << 8 | chip_8.memory[chip_8.pc + 1];

	switch (chip_8.opcode & 0xF000)
	{
	case 0x0000:
		switch (chip_8.opcode & 0x000F)
		{
		case 0x0000: // 0x00E0: Clears the screen
			for (int i = 0; i < 2048; ++i)
				chip_8.gfx[i] = 0x0;
			chip_8.drawFlag = TRUE;
			chip_8.pc += 2;
			break;

		case 0x000E: // 0x00EE: Returns from subroutine
			--chip_8.sp;			// 16 levels of stack, decrease stack pointer to prevent overwrite
			chip_8.pc = chip_8.stack[chip_8.sp];	// Put the stored return address from the stack back into the program counter
			chip_8.pc += 2;		// Don't forget to increase the program counter!
			break;

		default:
			printf("Unknown opcode [0x0000]: 0x%X\n", chip_8.opcode);
		}
		break;

	case 0x1000: // 0x1NNN: Jumps to address NNN
		chip_8.pc = chip_8.opcode & 0x0FFF;
		break;

	case 0x2000: // 0x2NNN: Calls subroutine at NNN.
		chip_8.stack[chip_8.sp] = chip_8.pc;			// Store current address in stack
		++chip_8.sp;					// Increment stack pointer
		chip_8.pc = chip_8.opcode & 0x0FFF;	// Set the program counter to the address at NNN
		break;

	case 0x3000: // 0x3XNN: Skips the next instruction if VX equals NN
		if (chip_8.V[(chip_8.opcode & 0x0F00) >> 8] == (chip_8.opcode & 0x00FF))
			chip_8.pc += 4;
		else
			chip_8.pc += 2;
		break;

	case 0x4000: // 0x4XNN: Skips the next instruction if VX doesn't equal NN
		if (chip_8.V[(chip_8.opcode & 0x0F00) >> 8] != (chip_8.opcode & 0x00FF))
			chip_8.pc += 4;
		else
			chip_8.pc += 2;
		break;

	case 0x5000: // 0x5XY0: Skips the next instruction if VX equals VY.
		if (chip_8.V[(chip_8.opcode & 0x0F00) >> 8] == chip_8.V[(chip_8.opcode & 0x00F0) >> 4])
			chip_8.pc += 4;
		else
			chip_8.pc += 2;
		break;

	case 0x6000: // 0x6XNN: Sets VX to NN.
		chip_8.V[(chip_8.opcode & 0x0F00) >> 8] = chip_8.opcode & 0x00FF;
		chip_8.pc += 2;
		break;

	case 0x7000: // 0x7XNN: Adds NN to VX.
		chip_8.V[(chip_8.opcode & 0x0F00) >> 8] += chip_8.opcode & 0x00FF;
		chip_8.pc += 2;
		break;

	case 0x8000:
		switch (chip_8.opcode & 0x000F)
		{
		case 0x0000: // 0x8XY0: Sets VX to the value of VY
			chip_8.V[(chip_8.opcode & 0x0F00) >> 8] = chip_8.V[(chip_8.opcode & 0x00F0) >> 4];
			chip_8.pc += 2;
			break;

		case 0x0001: // 0x8XY1: Sets VX to "VX OR VY"
			chip_8.V[(chip_8.opcode & 0x0F00) >> 8] |= chip_8.V[(chip_8.opcode & 0x00F0) >> 4];
			chip_8.pc += 2;
			break;

		case 0x0002: // 0x8XY2: Sets VX to "VX AND VY"
			chip_8.V[(chip_8.opcode & 0x0F00) >> 8] &= chip_8.V[(chip_8.opcode & 0x00F0) >> 4];
			chip_8.pc += 2;
			break;

		case 0x0003: // 0x8XY3: Sets VX to "VX XOR VY"
			chip_8.V[(chip_8.opcode & 0x0F00) >> 8] ^= chip_8.V[(chip_8.opcode & 0x00F0) >> 4];
			chip_8.pc += 2;
			break;

		case 0x0004: // 0x8XY4: Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't
			if (chip_8.V[(chip_8.opcode & 0x00F0) >> 4] > (0xFF - chip_8.V[(chip_8.opcode & 0x0F00) >> 8]))
				chip_8.V[0xF] = 1; //carry
			else
				chip_8.V[0xF] = 0;
			chip_8.V[(chip_8.opcode & 0x0F00) >> 8] += chip_8.V[(chip_8.opcode & 0x00F0) >> 4];
			chip_8.pc += 2;
			break;

		case 0x0005: // 0x8XY5: VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't
			if (chip_8.V[(chip_8.opcode & 0x00F0) >> 4] > chip_8.V[(chip_8.opcode & 0x0F00) >> 8])
				chip_8.V[0xF] = 0; // there is a borrow
			else
				chip_8.V[0xF] = 1;
			chip_8.V[(chip_8.opcode & 0x0F00) >> 8] -= chip_8.V[(chip_8.opcode & 0x00F0) >> 4];
			chip_8.pc += 2;
			break;

		case 0x0006: // 0x8XY6: Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift
			chip_8.V[0xF] = chip_8.V[(chip_8.opcode & 0x0F00) >> 8] & 0x1;
			chip_8.V[(chip_8.opcode & 0x0F00) >> 8] >>= 1;
			chip_8.pc += 2;
			break;

		case 0x0007: // 0x8XY7: Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't
			if (chip_8.V[(chip_8.opcode & 0x0F00) >> 8] > chip_8.V[(chip_8.opcode & 0x00F0) >> 4])	// VY-VX
				chip_8.V[0xF] = 0; // there is a borrow
			else
				chip_8.V[0xF] = 1;
			chip_8.V[(chip_8.opcode & 0x0F00) >> 8] = chip_8.V[(chip_8.opcode & 0x00F0) >> 4] - chip_8.V[(chip_8.opcode & 0x0F00) >> 8];
			chip_8.pc += 2;
			break;

		case 0x000E: // 0x8XYE: Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift
			chip_8.V[0xF] = chip_8.V[(chip_8.opcode & 0x0F00) >> 8] >> 7;
			chip_8.V[(chip_8.opcode & 0x0F00) >> 8] <<= 1;
			chip_8.pc += 2;
			break;

		default:
			printf("Unknown opcode [0x8000]: 0x%X\n", chip_8.opcode);
		}
		break;

	case 0x9000: // 0x9XY0: Skips the next instruction if VX doesn't equal VY
		if (chip_8.V[(chip_8.opcode & 0x0F00) >> 8] != chip_8.V[(chip_8.opcode & 0x00F0) >> 4])
			chip_8.pc += 4;
		else
			chip_8.pc += 2;
		break;

	case 0xA000: // ANNN: Sets I to the address NNN
		chip_8.I = chip_8.opcode & 0x0FFF;
		chip_8.pc += 2;
		break;

	case 0xB000: // BNNN: Jumps to the address NNN plus V0
		chip_8.pc = (chip_8.opcode & 0x0FFF) + chip_8.V[0];
		break;

	case 0xC000: // CXNN: Sets VX to a random number and NN
		chip_8.V[(chip_8.opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (chip_8.opcode & 0x00FF);
		chip_8.pc += 2;
		break;

	case 0xD000: // DXYN: Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels.
				 // Each row of 8 pixels is read as bit-coded starting from memory location I;
				 // I value doesn't change after the execution of this instruction.
				 // VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn,
				 // and to 0 if that doesn't happen
	{
		unsigned short x = chip_8.V[(chip_8.opcode & 0x0F00) >> 8];
		unsigned short y = chip_8.V[(chip_8.opcode & 0x00F0) >> 4];
		unsigned short height = chip_8.opcode & 0x000F;
		unsigned short pixel;

		chip_8.V[0xF] = 0;
		for (int yline = 0; yline < height; yline++)
		{
			pixel = chip_8.memory[chip_8.I + yline];
			for (int xline = 0; xline < 8; xline++)
			{
				if ((pixel & (0x80 >> xline)) != 0)
				{
					if (chip_8.gfx[(x + xline + ((y + yline) * 64))] == 1)
					{
						chip_8.V[0xF] = 1;
					}
					chip_8.gfx[x + xline + ((y + yline) * 64)] ^= 1;
				}
			}
		}

		chip_8.drawFlag = TRUE;
		chip_8.pc += 2;
	}
	break;

	case 0xE000:
		switch (chip_8.opcode & 0x00FF)
		{
		case 0x009E: // EX9E: Skips the next instruction if the key stored in VX is pressed
			if (chip_8.key[chip_8.V[(chip_8.opcode & 0x0F00) >> 8]] != 0)
				chip_8.pc += 4;
			else
				chip_8.pc += 2;
			break;

		case 0x00A1: // EXA1: Skips the next instruction if the key stored in VX isn't pressed
			if (chip_8.key[chip_8.V[(chip_8.opcode & 0x0F00) >> 8]] == 0)
				chip_8.pc += 4;
			else
				chip_8.pc += 2;
			break;

		default:
			printf("Unknown opcode [0xE000]: 0x%X\n", chip_8.opcode);
		}
		break;

	case 0xF000:
		switch (chip_8.opcode & 0x00FF)
		{
		case 0x0007: // FX07: Sets VX to the value of the delay timer
			chip_8.V[(chip_8.opcode & 0x0F00) >> 8] = chip_8.delay_timer;
			chip_8.pc += 2;
			break;

		case 0x000A: // FX0A: A key press is awaited, and then stored in VX
		{
			int keyPress = FALSE;

			for (int i = 0; i < 16; ++i)
			{
				if (chip_8.key[i] != 0)
				{
					chip_8.V[(chip_8.opcode & 0x0F00) >> 8] = i;
					keyPress = TRUE;
				}
			}

			// If we didn't received a keypress, skip this cycle and try again.
			if (!keyPress)
				return;

			chip_8.pc += 2;
		}
		break;

		case 0x0015: // FX15: Sets the delay timer to VX
			chip_8.delay_timer = chip_8.V[(chip_8.opcode & 0x0F00) >> 8];
			chip_8.pc += 2;
			break;

		case 0x0018: // FX18: Sets the sound timer to VX
			chip_8.sound_timer = chip_8.V[(chip_8.opcode & 0x0F00) >> 8];
			chip_8.pc += 2;
			break;

		case 0x001E: // FX1E: Adds VX to I
			if (chip_8.I + chip_8.V[(chip_8.opcode & 0x0F00) >> 8] > 0xFFF)	// VF is set to 1 when range overflow (I+VX>0xFFF), and 0 when there isn't.
				chip_8.V[0xF] = 1;
			else
				chip_8.V[0xF] = 0;
			chip_8.I += chip_8.V[(chip_8.opcode & 0x0F00) >> 8];
			chip_8.pc += 2;
			break;

		case 0x0029: // FX29: Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font
			chip_8.I = chip_8.V[(chip_8.opcode & 0x0F00) >> 8] * 0x5;
			chip_8.pc += 2;
			break;

		case 0x0033: // FX33: Stores the Binary-coded decimal representation of VX at the addresses I, I plus 1, and I plus 2
			chip_8.memory[chip_8.I] = chip_8.V[(chip_8.opcode & 0x0F00) >> 8] / 100;
			chip_8.memory[chip_8.I + 1] = (chip_8.V[(chip_8.opcode & 0x0F00) >> 8] / 10) % 10;
			chip_8.memory[chip_8.I + 2] = (chip_8.V[(chip_8.opcode & 0x0F00) >> 8] % 100) % 10;
			chip_8.pc += 2;
			break;

		case 0x0055: // FX55: Stores V0 to VX in memory starting at address I
			for (int i = 0; i <= ((chip_8.opcode & 0x0F00) >> 8); ++i)
				chip_8.memory[chip_8.I + i] = chip_8.V[i];

			// On the original interpreter, when the operation is done, I = I + X + 1.
			chip_8.I += ((chip_8.opcode & 0x0F00) >> 8) + 1;
			chip_8.pc += 2;
			break;

		case 0x0065: // FX65: Fills V0 to VX with values from memory starting at address I
			for (int i = 0; i <= ((chip_8.opcode & 0x0F00) >> 8); ++i)
				chip_8.V[i] = chip_8.memory[chip_8.I + i];

			// On the original interpreter, when the operation is done, I = I + X + 1.
			chip_8.I += ((chip_8.opcode & 0x0F00) >> 8) + 1;
			chip_8.pc += 2;
			break;

		default:
			printf("Unknown opcode [0xF000]: 0x%X\n", chip_8.opcode);
		}
		break;

	default:
		printf("Unknown opcode: 0x%X\n", chip_8.opcode);
	}
	if (chip_8.delay_timer > 0)
		--chip_8.delay_timer;

	 if (chip_8.sound_timer > 0)
		--chip_8.sound_timer;
}

void debugRender(void)
{
	for (int y = 0; y < 64; ++y)
	{
		for (int x = 0; x < 32; ++x)
		{
			if (chip_8.gfx[(64 * y) + x] == 0)
				printf("O");
			else
				printf(" ");
		}
		printf("\n");
	}
	printf("\n");
}


int loadApplication(const char *filename)
{
	init();
	printf("Loading: %s\n", filename);


	// Open file
	FILE * pFile = fopen(filename, "rb");
	if (pFile == NULL)
	{
		fputs("File error", stderr);
		return FALSE;
	}

	// Check file size
	fseek(pFile, 0, SEEK_END);
	long lSize = ftell(pFile);
	rewind(pFile);
	printf("Filesize: %d\n", (int)lSize);

	// Allocate memory to contain the whole file
	char * buffer = (char*)malloc(sizeof(char) * lSize);
	if (buffer == NULL)
	{
		fputs("Memory error", stderr);
		return FALSE;
	}

	// Copy the file into the buffer
	size_t result = fread(buffer, 1, lSize, pFile);
	if (result != lSize)
	{
		fputs("Reading error", stderr);
		return FALSE;
	}

	// Copy buffer to Chip8 memory
	if ((4096 - 512) > lSize)
	{
		for (int i = 0; i < lSize; ++i)
			chip_8.memory[i + 512] = buffer[i];
	}
	else
		printf("Error: ROM too big for memory");

	// Close file, free buffer
	fclose(pFile);
	free(buffer);

	return TRUE;

}






