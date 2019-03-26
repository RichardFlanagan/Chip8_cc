#include "SDL2/SDL.h"
#include "Chip8.h"
#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <streambuf>
#include <vector>


const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;


void load_rom_file(Chip8 *chip8, std::string rom_file){
	std::ifstream is (rom_file, std::ifstream::binary);

	if (is){
	    // Get length of file:
	    is.seekg(0, is.end);
	    int length = is.tellg();
	    is.seekg(0, is.beg);
	
		char* buffer = new char[length];
	   	is.read(buffer,length);

	   	uint8_t* buffer_int = new uint8_t[length];
	    for (int i = 0; i < length; ++i){
			buffer_int[i] = (uint8_t)buffer[i];			
	   	}

	    chip8->set_memory_block(
	    	(uint16_t) 0x0200,
	    	buffer_int,
	    	(uint16_t) length);
	
	    delete[] buffer;
	    delete[] buffer_int;
		is.close();
	}
}

void load_sprites_file(Chip8 *chip8){
	std::string spr = "../src/sprites.txt";
	std::ifstream is(spr, std::ifstream::binary);

	if (is){
	    // Get length of file:
	    is.seekg(0, is.end);
	    int length = is.tellg();
	    is.seekg(0, is.beg);
	
		char* buffer = new char[length];
	   	is.read(buffer,length);

	   	uint8_t* buffer_int = new uint8_t[length];
	    for (int i = 0; i < length; ++i){
			buffer_int[i] = (uint8_t)buffer[i];
	   	}

	    chip8->set_memory_block(
	    	(uint16_t) 0x0000,
	    	buffer_int,
	    	(uint16_t) length);
	
	    delete[] buffer;
	    delete[] buffer_int;
		is.close();
	}else {
	 	std::cout << "failed to load sprites" << std::endl;

	}
}



void print_ram(Chip8 *chip8){
	int per_row = 32;
	
	for (int i = 0; i < 4096/per_row; ++i){

		if (i*per_row < 10){
			std::cout << "   ";
		} else if (i*per_row < 100){
			std::cout << "  ";
		} else if (i*per_row < 1000){
			std::cout << " ";
		}
		std::cout << std::dec << i*per_row << " | ";


		for (int j = 0; j < per_row; ++j){
			uint8_t op = (uint8_t)chip8->get_at_memory_address((i*per_row+j));
			
			if(op < 16){
				std::cout << "0" << std::hex << (int)op;
			} else if(op == 0){
				std::cout << "00";
			} else{
				std::cout << std::hex << (int)op;
			}
			if(j%2 == 1){
				std::cout << " ";
			}
		}
		std::cout << std::endl;
	}
}



Uint32 get_pixel32( SDL_Surface *surface, int x, int y ){
    //Convert the pixels to 32 bit
    Uint32 *pixels = (Uint32 *)surface->pixels;
    
    //Get the requested pixel
    return pixels[ ( y * surface->w ) + x ];
}


void set_pixel32( SDL_Surface *surface, int x, int y, Uint32 pixel ){
    //Convert the pixels to 32 bit
    Uint32 *pixels = (Uint32 *)surface->pixels;
    
    //Set the pixel
    pixels[ ( y * surface->w ) + x ] = pixel;
}


void draw_chip8_display(SDL_Surface *surface, Chip8 *chip8){
	SDL_PixelFormat *fmt;
	fmt = surface->format;

    if( SDL_MUSTLOCK(surface)){
        SDL_LockSurface(surface);
    }


    uint8_t *display = chip8->get_display();
    int display_size = chip8->get_display_width() * chip8->get_display_height();

    for (int i = 0; i < display_size; ++i){
	Uint32 pix = SDL_MapRGBA(fmt, 0, 255*(int)display[i], 0, 1);
    	set_pixel32(surface, 
    		i%chip8->get_display_width(),
    		i/chip8->get_display_width(),
    		pix);

    }


    if(SDL_MUSTLOCK(surface)){
        SDL_UnlockSurface(surface);
    }

}

void draw_chip8_display(SDL_Surface *surface, Chip8 *chip8, int display_ratio){
	SDL_PixelFormat *fmt;
	fmt = surface->format;

    if( SDL_MUSTLOCK(surface)){
        SDL_LockSurface(surface);
    }


    uint8_t *display = chip8->get_display();
    int display_size = chip8->get_display_width() * chip8->get_display_height();

    for (int i = 0; i < display_size; ++i){
		Uint32 pix = SDL_MapRGBA(fmt, 0, 255*(bool)display[i], 0, 1);

		int pix_x = i%chip8->get_display_width()*display_ratio;
		int pix_y = i/chip8->get_display_width()*display_ratio;

		for (int j = 0; j < display_ratio; ++j){
			for (int k = 0; k < display_ratio; ++k){
				set_pixel32(surface, 
		    		pix_x+j,
		    		pix_y+k,
		    		pix);
			}
		}
    }


    if(SDL_MUSTLOCK(surface)){
        SDL_UnlockSurface(surface);
    }

}

void draw_all_sprites(Chip8 *chip8){
	for (int i = 0; i < 16; ++i){
		uint8_t x = 8*i;
		uint8_t y = i*8/chip8->get_display_width() * 5;
		chip8->draw_sprite(0x00+5*i, 5, x,y);		
	}
}

int main(int argc, char* args[]){

	//The window we'll be rendering to
	SDL_Window* window = NULL;

	//The surface contained by the window
	SDL_Surface* screenSurface = NULL;

	// Chip8 stuff
	Chip8 chip8;
	chip8.fill_display(0);
	int display_ratio = 4;
	SDL_Rect chip8_location{200, 200, chip8.get_display_width()*display_ratio, chip8.get_display_height()*display_ratio};

	load_sprites_file(&chip8);
	// draw_all_sprites(&chip8);

	load_rom_file(&chip8, "../roms/programs/IBM Logo.ch8");
	print_ram(&chip8);



	//The image we will load and show on the screen
	SDL_Surface* gameDisplaySurface = NULL;
	gameDisplaySurface = SDL_CreateRGBSurface(0, chip8.get_display_width()*display_ratio, chip8.get_display_height()*display_ratio, 32, 0, 0, 0, 0);


	//Initialize SDL
	if( SDL_Init(SDL_INIT_VIDEO) < 0){
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		return 1;
	} 
	else {
		//Create window
		window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if(window == NULL){
			printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
			return 1;
		}
		// else{
		// 	//Get window surface
		// 	screenSurface = SDL_GetWindowSurface(window);

		// 	//Fill the surface black
		// 	SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0x30, 0x30, 0x30));
		// 	SDL_FillRect(gameDisplaySurface, NULL, SDL_MapRGB(gameDisplaySurface->format, 0x00, 0x00, 0x00));

		// 	// Draw the Chip8 screen
  //           draw_chip8_display(gameDisplaySurface, &chip8, display_ratio);
  //           SDL_BlitSurface(gameDisplaySurface, NULL, screenSurface, &chip8_location);

		// 	//Update the surface
		// 	SDL_UpdateWindowSurface(window);

		// 	//Wait two seconds
		// 	SDL_Delay(1000);
		// }
	}

	//Get window surface
	screenSurface = SDL_GetWindowSurface(window);

	int run = 1;
	while(run){

		//Fill the surface black
		SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0x30, 0x30, 0x30));
		SDL_FillRect(gameDisplaySurface, NULL, SDL_MapRGB(gameDisplaySurface->format, 0x00, 0x00, 0x00));

		run = chip8.execute_next_op();

		// Draw the Chip8 screen
        draw_chip8_display(gameDisplaySurface, &chip8, display_ratio);
        SDL_BlitSurface(gameDisplaySurface, NULL, screenSurface, &chip8_location);

		//Update the surface
		SDL_UpdateWindowSurface(window);

		//Wait two seconds
		// SDL_Delay(1000);
	}

	//Destroy window
	SDL_DestroyWindow(window);

	//Quit SDL subsystems
	SDL_Quit();

	return 0;
}
