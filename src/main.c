#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_scancode.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

const int SCREEN_WIDTH = 600;
const int SCREEN_HEIGHT = 500;

const int GRID_WIDTH = 60;
const int GRID_HEIGHT = 50;

void randomFill(float *grid) {
  for(int y = 0; y < GRID_HEIGHT; y++)
    for(int x = 0; x < GRID_WIDTH; x++)
      grid[y * GRID_WIDTH + x] = (float)rand() / (float)RAND_MAX; 
}

int main() {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
    exit(1);
  }


  SDL_Window* window = SDL_CreateWindow
	(
		"Window",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		SCREEN_WIDTH, SCREEN_HEIGHT,
		SDL_WINDOW_SHOWN
	);

	SDL_Renderer* renderer = SDL_CreateRenderer
	(
		window,
		-1,
		SDL_RENDERER_ACCELERATED
	);

	SDL_Texture* texture = SDL_CreateTexture
	(
		renderer,
		SDL_PIXELFORMAT_RGBA8888,
		SDL_TEXTUREACCESS_STREAMING,
		SCREEN_WIDTH, SCREEN_HEIGHT
	);

  uint32_t pixels[SCREEN_WIDTH * SCREEN_HEIGHT];
  float grid[SCREEN_WIDTH / 10 * SCREEN_HEIGHT / 10];
  for(int i = 0; i < SCREEN_HEIGHT * SCREEN_WIDTH; i++) {
    pixels[i] = 0x0000FFFF;
  }

	int running = 1;  
  while(running) {

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT)
            running = 0;
    }

    const Uint8* keyboard = SDL_GetKeyboardState(NULL);

  	if (keyboard[SDL_SCANCODE_Q]) running = 0;
  	if (keyboard[SDL_SCANCODE_SPACE]) randomFill(grid);


  	for(int y = 0; y < SCREEN_HEIGHT; y++)
  	{
  	  for(int x = 0; x < SCREEN_WIDTH; x++)
  	  {
  	    int gridX = x / 10;
  	    int gridY = y / 10;
  	    float shade = grid[gridY * GRID_WIDTH + gridX];
  	    uint8_t r = 0xff * shade;
  	    uint8_t g = 0xff * shade;
  	    uint8_t b = 0xff * shade;
  	    uint32_t c = (r << 24) | (g << 16) | (b << 8) | (0xff << 0);
  	    pixels[y * SCREEN_WIDTH + x] = c;
  	  }
  	}
  	
		SDL_UpdateTexture(texture, NULL, pixels,  SCREEN_WIDTH * sizeof(uint32_t));
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
  }

  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
