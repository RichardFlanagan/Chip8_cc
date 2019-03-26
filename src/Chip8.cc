#include <algorithm>
#include "Chip8.h"
#include <bitset>
#include <iostream>
#include <string>
#include <sstream>
#include <chrono>
#include <thread>



Chip8::Chip8(){
	init_registers();
}

Chip8::~Chip8(){}

void Chip8::init_registers(){
	std::fill(memory, memory+4096, 0);		// RAM
	std::fill(V, V+16, 0);					// Multi-purpose registers. V[15] is reserved
	I = 0;									// Address register
	delay_timer = 0;						// Delay timer
	sound_timer = 0;						// Sound timer
	PC = 0x200;								// Program counter
	SP = 0;									// Stack pointer
	std::fill(stack, stack+16, 0);			// Call stack
	fill_display(0);						// Game display
	debug = 0xFF;							// Debug mode flags
}


uint8_t Chip8::get_at_memory_address(uint16_t address){
	return memory[address];
}
void Chip8::set_memory_address(uint16_t address, uint8_t value){
	// TODO
}
void Chip8::set_memory_block(uint16_t address, uint8_t *value, uint16_t length){
	for (int i = 0; i < length; ++i){
		memory[address+i] = value[i];
	}
}

uint8_t Chip8::get_V(uint8_t index){
	return V[index];
}
void Chip8::set_V(uint8_t index, uint8_t value){
	V[index] = value;
}

uint16_t Chip8::get_I(){
	return I;
}
void Chip8::set_I(uint16_t value){
	I = value;
}

uint8_t Chip8::get_delay_timer(){
	return delay_timer;
}
void Chip8::set_delay_timer(uint8_t value){
	delay_timer = value;
}

uint8_t Chip8::get_sound_timer(){
	return sound_timer;
}
void Chip8::set_sound_timer(uint8_t value){
	sound_timer = value;
}

uint16_t Chip8::get_PC(){
	return PC;
}
void Chip8::set_PC(uint16_t address){
	PC = address;
}

uint8_t Chip8::get_SP(){
	return SP;
}
void Chip8::set_SP(uint8_t value){
	SP = value;
}
void Chip8::inc_SP(){
	SP++;
}
void Chip8::dec_SP(){
	SP--;
}

uint16_t* Chip8::get_stack(){
	return stack;
}
void Chip8::set_stack(uint8_t index, uint16_t address){
	stack[index] = address;
}
void Chip8::push_stack(uint16_t address){
	stack[SP] = address;
	inc_SP();
}
uint16_t Chip8::pop_stack(){
	return stack[SP];
	set_stack(SP, 0);
	dec_SP();
}

uint8_t* Chip8::get_display(){
	return display;
}
void Chip8::set_display_pixel(uint16_t index, uint8_t value){
	display[index] = value;
}
void Chip8::set_display_block(uint16_t index, uint8_t value, uint16_t length){
	// TODO
}
void Chip8::fill_display(uint8_t value){
	int display_size = get_display_width() * get_display_height();
	std::fill(display, display+display_size, value);
}
int Chip8::get_display_width(){
	return 64;
}
int Chip8::get_display_height(){
	return 32;
}


void Chip8::draw_sprite(uint16_t address, uint8_t length, uint8_t x, uint8_t y){
	
	for (int j = 0; j < length; ++j){

		uint8_t aa = get_at_memory_address(address+j);
		std::string binary = std::bitset<8>(aa).to_string();

		for (int i = 0; i < 8; ++i){
			int pixel_index = (x+y*get_display_width()) + (i + j*get_display_width());
			if (binary[i] == '1'){
				display[pixel_index] = (int)binary[i];
			}
		}
	}
}

std::string debug_get_op_hex(uint16_t op){		
	std::stringstream ss;

	if((op & 0xFFF0) == 0){
		ss << "000";
	}
	else if((op & 0xFF00) == 0){
		ss << "00";
	}
	else if((op & 0xF000) == 0){
		ss << "0";
	}
	ss << std::hex << op;

	return ss.str();
}

// std::string debug_get_ram_hex(Chip8 *chip8){
// 	int per_row = 32;
// 	std::stringstream ss;
	
// 	for (int i = 0; i < 4096/per_row; ++i){

// 		if (i*per_row < 10){
// 			ss << "   ";
// 		} else if (i*per_row < 100){
// 			ss << "  ";
// 		} else if (i*per_row < 1000){
// 			ss << " ";
// 		}
// 		ss << std::dec << i*per_row << " | ";


// 		for (int j = 0; j < per_row; ++j){
// 			uint8_t op = (uint8_t)chip8->get_at_memory_address((i*per_row+j));
			
// 			if(op < 16){
// 				ss << "0" << std::hex << (int)op;
// 			} else if(op == 0){
// 				ss << "00";
// 			} else{
// 				ss << std::hex << (int)op;
// 			}
// 			if(j%2 == 1){
// 				ss << " ";
// 			}
// 		}
// 		ss << std::endl;
// 	}

// 	return ss.str();
// }


std::string debug_get_op_bin(uint16_t op){
	std::stringstream ss;
	std::bitset<16> x(op);
	ss << x;
	return ss.str();
}

void Chip8::start(){
	// Initialize op, our current instruction.
	uint16_t op = 0;
	
	// std::cout << debug_get_ram_hex(this) << std::endl;

	do{	
		// Assign op to the current bytes at the program counter. 
		// We shift the first byte and append the second to the new space.
		op = (memory[PC] << 8) | memory[PC+1];
		
		// if (debug){
			std::cout
				<< debug_get_op_hex(op);
		// 		<< " | "
		// 		<< debug_get_op_bin(op)
		// 		<< std::endl;
		// }

		// Interpret and carry out the instruction
		interpret(op);

		// Increment the program counter by 1 op (2 bytes)
		PC += 2;

	} while(op != 0); // Continue until we reach null bytes.

}

int Chip8::execute_next_op(){
	// Initialize op, our current instruction.
	// Assign op to the current bytes at the program counter. 
	// We shift the first byte and append the second to the new space.
	uint16_t op = (memory[PC] << 8) | memory[PC+1];
		
	std::cout << debug_get_op_hex(op);

	// If we reach a NULL op, return 0 to signify end of exec
	if (op == 0){
		return 0;
	}
		
	// Interpret and carry out the instruction
	interpret(op);

	// Increment the program counter by 1 op (2 bytes)
	PC += 2;

	// Debug pause
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

	// Return 1 to signify continued operation
	return 1;

}

void Chip8::interpret(uint16_t op){

	// 0000 - NULL
	if (op == 0){
		std::cout << " : NULL";
	}

	// 00E0 - CLS
	// Clear the display.
	else if ((op & 0xFFFF) == 0x00E0){
		std::cout << " : Clear the display";
		fill_display(0);
	} 

	// 00EE - RET
	// Return from a subroutine.
	else if (op == 0x00EE){
		std::cout << " : Return from a subroutine";
	}

	// 0nnn - SYS addr
	// Jump to a machine code routine at nnn.
	else if ((op & 0xF000) == 0x0){
		uint16_t addr = op & 0x0FFF;
		std::cout << " : Jump to a machine code routine at " << addr;
		// This instruction is only used on the old computers on 
		// which Chip-8 was originally implemented. It is ignored 
		// by modern interpreters.
	}

	// 1nnn - JP addr
	// Jump to location nnn.
	else if ((op & 0xF000) == 0x1000){
		uint16_t addr = op & 0x0FFF;
		std::cout << " : Jump to location " << addr;

		PC = addr;
	}


	// 2nnn - CALL addr
	// Call subroutine at nnn.
	else if ((op & 0xF000) == 0x2000){
		uint16_t addr = op & 0x0FFF;
		std::cout << " : Call subroutine at " << addr;
	}	

	// 3xkk - SE Vx, byte
	// Skip next instruction if Vx = kk.
	else if ((op & 0xF000) == 0x3000){
		uint8_t x  = (op & 0x0F00) >> 8;
		uint8_t kk = op & 0x00FF;

		std::cout << " : Skip next instruction if Vx = kk";

		if(V[x] == kk){
			PC += 2;
		}
	}

	// 4xkk - SNE Vx, byte
	// Skip next instruction if Vx != kk.
	else if ((op & 0xF000) == 0x4000){
		uint8_t x  = (op & 0x0F00) >> 8;
		uint8_t kk = op & 0x00FF;

		std::cout << " : Skip next instruction if Vx != kk";

		if(V[x] != kk){
			PC += 2;
		}
	}

	// 5xy0 - SE Vx, Vy
	// Skip next instruction if Vx = Vy.
	else if ((op & 0xF00F) == 0x5000){
		uint8_t x = (op & 0x0F00) >> 8;
		uint8_t y = (op & 0x00F0) >> 4;

		std::cout << " : Skip next instruction if Vx = Vy";
	}	

	// 6xkk - LD Vx, byte
	// Set Vx = kk.
	else if ((op & 0xF000) == 0x6000){
		uint8_t x  = (op & 0x0F00) >> 8;
		uint8_t kk = (op & 0x00FF);

		std::cout << " : Set Vx = kk";
		std::cout << " | V" << (int)x << " = " << (int)kk << std::endl;

		V[x] = kk;

		// if (debug){
		// 	std::ios_base::fmtflags f( std::cout.flags() );
		// 	for (int i = 0; i < 16; ++i){
		// 		std::cout << "V" << i << " = " << std::dec << (int)V[i] << std::endl;
		// 	}
		// 	std::cout.flags( f );
		// }
	}

	// 7xkk - ADD Vx, byte
	// Set Vx = Vx + kk.
	else if ((op & 0xF000) == 0x7000){
		uint8_t x  = (op & 0x0F00) >> 8;
		uint8_t kk = op & 0x00FF;

		std::cout << " : Set Vx = Vx + kk" << "@V" << (int)x << "@" << (int)kk;
		// std::cout << "#" << (int)V[x] << std::endl;
		V[x] = V[x] + kk;
		// std::cout << "#" << (int)V[x] << std::endl;

	}

	// 8xy0 - LD Vx, Vy
	// Set Vx = Vy.
	else if ((op & 0xF00F) == 0x8000){
		uint8_t x = (op & 0x0F00) >> 8;
		uint8_t y = (op & 0x00F0) >> 4;

		std::cout << " : Set Vx = Vy";

		V[x] = V[y];
	}

	// 8xy1 - OR Vx, Vy
	// Set Vx = Vx OR Vy.
	else if ((op & 0xF00F) == 0x8001){
		uint8_t x = (op & 0x0F00) >> 8;
		uint8_t y = (op & 0x00F0) >> 4;

		std::cout << " : Set Vx = Vx OR Vy";
	}

	// 8xy2 - AND Vx, Vy
	// Set Vx = Vx AND Vy.
	else if ((op & 0xF00F) == 0x8002){
		uint8_t x = (op & 0x0F00) >> 8;
		uint8_t y = (op & 0x00F0) >> 4;

		std::cout << " : Set Vx = Vx AND Vy";
	}

	// 8xy3 - XOR Vx, Vy
	// Set Vx = Vx XOR Vy.
	else if ((op & 0xF00F) == 0x8003){
		uint8_t x = (op & 0x0F00) >> 8;
		uint8_t y = (op & 0x00F0) >> 4;

		std::cout << " : Set Vx = Vx XOR Vy";
	}

	// 8xy4 - ADD Vx, Vy
	// Set Vx = Vx + Vy, set VF = carry.
	else if ((op & 0xF00F) == 0x8004){
		uint8_t x = (op & 0x0F00) >> 8;
		uint8_t y = (op & 0x00F0) >> 4;

		std::cout << " : Set Vx = Vx + Vy, set VF = carry";
	}

	// 8xy5 - SUB Vx, Vy
	// Set Vx = Vx - Vy, set VF = NOT borrow.
	else if ((op & 0xF00F) == 0x8005){
		uint8_t x = (op & 0x0F00) >> 8;
		uint8_t y = (op & 0x00F0) >> 4;

		std::cout << " : Set Vx = Vx - Vy, set VF = NOT borrow";
	}

	// 8xy6 - SHR Vx {, Vy}
	// Set Vx = Vx SHR 1.
	else if ((op & 0xF00F) == 0x8006){
		uint8_t x = (op & 0x0F00) >> 8;
		uint8_t y = (op & 0x00F0) >> 4;

		std::cout << " : Set Vx = Vx + kk";
	}

	// 8xy7 - SUBN Vx, Vy
	// Set Vx = Vy - Vx, set VF = NOT borrow.
	else if ((op & 0xF00F) == 0x8007){
		uint8_t x = (op & 0x0F00) >> 8;
		uint8_t y = (op & 0x00F0) >> 4;

		std::cout << " : Set Vx = Vy - Vx, set VF = NOT borrow";
	}

	// 8xyE - SHL Vx {, Vy}
	// Set Vx = Vx SHL 1.
	else if ((op & 0xF00F) == 0x800E){
		uint8_t x = (op & 0x0F00) >> 8;
		uint8_t y = (op & 0x00F0) >> 4;

		std::cout << " : Set Vx = Vx SHL 1";
	}

	// 9xy0 - SNE Vx, Vy
	// Skip next instruction if Vx != Vy.
	else if ((op & 0xF00F) == 0x9000){
		uint8_t x = (op & 0x0F00) >> 8;
		uint8_t y = (op & 0x00F0) >> 4;

		std::cout << " : Skip next instruction if Vx != Vy";
	}

	// Annn - LD I, addr
	// Set I = nnn.
	else if ((op & 0xF000) == 0xA000){
		uint16_t addr = op & 0x0FFF;

		std::cout << " : Set I = nnn";
		std::cout << std::dec << "@" << I << "@" << addr << "@" << (int)memory[I] << "@" << (int)memory[addr];


		I = addr;
	}

	// Bnnn - JP V0, addr
	// Jump to location nnn + V0.
	else if ((op & 0xF000) == 0xB000){
		uint16_t addr = op & 0x0FFF;

		std::cout << " : Jump to location nnn + V0";

		PC = addr + V[0];
	}

	// Cxkk - RND Vx, byte
	// Set Vx = random byte AND kk.
	else if ((op & 0xF000) == 0xC000){
		uint8_t x  = (op & 0x0F00) >> 8;
		uint8_t kk = op & 0x00FF;

		std::cout << " : Set Vx = random byte AND kk";
	}

	// Dxyn - DRW Vx, Vy, nibble
	// Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
	else if ((op & 0xF000) == 0xD000){
		uint8_t x = (op & 0x0F00) >> 8;
		uint8_t y = (op & 0x00F0) >> 4;
		uint8_t n = op & 0x000F;

		std::cout << " : Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision";

		for (int j = 0; j < n; ++j){

			uint8_t aa = get_at_memory_address(I+j);
			std::string binary = std::bitset<8>(aa).to_string();

			for (int i = 0; i < 8; ++i){
				int pixel_index = (V[x]+V[y]*get_display_width()) + (i + j*get_display_width());

				if((int)binary[i] && display[pixel_index]){
					V[0xF] = 1;
				}

				if (binary[i] == '1'){
					display[pixel_index] = (int)binary[i] ^ display[pixel_index];
				}
			}

			
		}

		std::cout << "\n" << (int)x << ":" << (int)y << " N=" << (int)n << std::endl;

		std::cout << debug_get_op_bin(op) << std::endl;
		std::cout << debug_get_op_bin(0x0F00) << std::endl;
		std::cout << (int)x << std::endl << std::endl;

		std::cout << debug_get_op_bin(op) << std::endl;
		std::cout << debug_get_op_bin(0x00F0) << std::endl;
		std::cout << (int)y << std::endl;




		// Draw lines
		// for (int i = 0; i < n; ++i){
		// 	int pos_offset = V[y]*get_display_width() + V[x];
		// 	uint8_t sprite_byte = memory[i];
		// 	std::string binary = std::bitset<8>(sprite_byte).to_string();

		// 	for (int j = 0; j < 8; ++j){
		// 		// int pixel_index = (x+y*get_display_width()) + (i + j*get_display_width());

		// 		int base_index = i*get_display_width() + j;

		// 		//
		// 		// LAST STOPPPED HERE
		// 		// DRAW ISNT WORKING CORRECTLY
		// 		//


		// 		if (binary[i] == '1'){
		// 			display[base_index+pos_offset] = (int)binary[i];
		// 		}
		// 	}


			// Draw bits
			// for (int j = 0; j < 8; ++j){
				// int base_index = i*get_display_width() + j;
				// int pos_offset = V[y]*get_display_width() + V[x];

				// int disp_pos = base_index + pos_offset;
				// int mem_pos = base_index + I;


				// if(display[disp_pos] && memory[mem_pos]){
				// 	V[0x0F] = 1;
				// }



			// 	display[disp_pos] = display[disp_pos] ^ memory[mem_pos];
			// }

		// }

		// draw_sprite(I, n, x, y);
	}

	// void Chip8::draw_sprite(uint16_t address, uint8_t length, uint8_t x, uint8_t y){
	
	// for (int j = 0; j < length; ++j){

	// 	uint8_t aa = get_at_memory_address(address+j);
	// 	std::string binary = std::bitset<8>(aa).to_string();

	// 	for (int i = 0; i < 8; ++i){
	// 		int pixel_index = (x+y*get_display_width()) + (i + j*get_display_width());
	// 		if (binary[i] == '1'){
	// 			display[pixel_index] = (int)binary[i];
	// 		}
	// 	}
	// }
// }


	// Ex9E - SKP Vx
	// Skip next instruction if key with the value of Vx is pressed.
	else if ((op & 0xF000) == 0xE09E){
		uint8_t x = (op & 0x0F00) >> 8;

		std::cout << " : Skip next instruction if key with the value of Vx is pressed";
	}

	// ExA1 - SKNP Vx
	// Skip next instruction if key with the value of Vx is not pressed.
	else if ((op & 0xF0FF) == 0xE0A1){
		uint8_t x = (op & 0x0F00) >> 8;

		std::cout << " : Skip next instruction if key with the value of Vx is not pressed";
	}

	// Fx07 - LD Vx, DT
	// Set Vx = delay timer value.
	else if ((op & 0xF0FF) == 0xF007){
		uint8_t x = (op & 0x0F00) >> 8;

		std::cout << " : Set Vx = delay timer value";
	}

	// Fx0A - LD Vx, K
	// Wait for a key press, store the value of the key in Vx.
	else if ((op & 0xF0FF) == 0xF00A){
		uint8_t x = (op & 0x0F00) >> 8;

		std::cout << " : Wait for a key press, store the value of the key in Vx";
	}

	// Fx15 - LD DT, Vx
	// Set delay timer = Vx.
	else if ((op & 0xF0FF) == 0xF015){
		uint8_t x = (op & 0x0F00) >> 8;

		std::cout << " : Set delay timer = Vx";
	}

	// Fx18 - LD ST, Vx
	// Set sound timer = Vx.
	else if ((op & 0xF0FF) == 0xF018){
		uint8_t x = (op & 0x0F00) >> 8;

		std::cout << " : Set sound timer = Vx";
	}

	// Fx1E - ADD I, Vx
	// Set I = I + Vx.
	else if ((op & 0xF0FF) == 0xF01E){
		uint8_t x = (op & 0x0F00) >> 8;

		std::cout << " : Set I = I + Vx";
	}

	// Fx29 - LD F, Vx
	// Set I = location of sprite for digit Vx.
	else if ((op & 0xF0FF) == 0xF029){
		uint8_t x = (op & 0x0F00) >> 8;

		std::cout << " : Set I = location of sprite for digit Vx";
	}

	// Fx33 - LD B, Vx
	// Store BCD representation of Vx in memory locations I, I+1, and I+2.
	else if ((op & 0xF0FF) == 0xF033){
		uint8_t x = (op & 0x0F00) >> 8;

		std::cout << " : Store BCD representation of Vx in memory locations I, I+1, and I+2";
	}

	// Fx55 - LD [I], Vx
	// Store registers V0 through Vx in memory starting at location I.
	else if ((op & 0xF0FF) == 0xF055){
		uint8_t x = (op & 0x0F00) >> 8;

		std::cout << " : Store registers V0 through Vx in memory starting at location I";
	}

	// Fx65 - LD Vx, [I]
	// Read registers V0 through Vx from memory starting at location I.
	else if ((op & 0xF0FF) == 0xF065){
		uint8_t x = (op & 0x0F00) >> 8;

		std::cout << " : Read registers V0 through Vx from memory starting at location I";
	}

	std::cout << std::endl;	
}