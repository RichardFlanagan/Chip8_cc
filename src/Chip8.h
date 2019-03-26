#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>
#include <string>

class Chip8{
private:
	uint8_t	 memory[4096];		// RAM
	uint8_t  V[16];				// Multi-purpose registers. V[15] is reserved
	uint16_t I;					// Address register
	uint8_t  delay_timer;		// Delay timer
	uint8_t  sound_timer;		// Sound timer
	uint16_t PC;				// Program counter
	uint8_t  SP;				// Stack pointer
	uint16_t stack[16];			// Call stack
	uint8_t  display[64*32];	// Game display
	uint8_t  debug;				// Debug mode flags

	void init_registers();

public:
	Chip8();
	~Chip8();

	uint8_t get_at_memory_address(uint16_t address);
	void set_memory_address(uint16_t address, uint8_t value);
	void set_memory_block(uint16_t address, uint8_t *value, uint16_t length);

	uint8_t get_V(uint8_t index);
	void set_V(uint8_t index, uint8_t value);

	uint16_t get_I();
	void set_I(uint16_t value);

	uint8_t get_delay_timer();
	void set_delay_timer(uint8_t value);

	uint8_t get_sound_timer();
	void set_sound_timer(uint8_t value);

	uint16_t get_PC();
	void set_PC(uint16_t value);

	uint8_t get_SP();
	void set_SP(uint8_t value);
	void inc_SP();
	void dec_SP();

	uint16_t* get_stack();
	void set_stack(uint8_t index, uint16_t address);
	void push_stack(uint16_t address);
	uint16_t pop_stack();

	uint8_t* get_display();
	void set_display_pixel(uint16_t index, uint8_t value);
	void set_display_block(uint16_t index, uint8_t value, uint16_t length);
	void fill_display(uint8_t value);
	int get_display_width();
	int get_display_height();

	void draw_sprite(uint16_t address, uint8_t length, uint8_t x, uint8_t y);
	void start();
	int execute_next_op();
	
	void interpret(uint16_t op);

};

#endif