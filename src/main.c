#include <SDL2/SDL.h>
#include <SDL2/SDL_scancode.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

const int SCREEN_WIDTH = 600;
const int SCREEN_HEIGHT = 500;

void randomFill(float *baseNoise) {
  for(int i = 0; i < SCREEN_WIDTH; i++)
      baseNoise[i] = (float)rand() / (float)RAND_MAX; 
}

void fractalNoise1D(float* heights, float* baseNoise) {
  for(int i = 0; i < SCREEN_WIDTH; i++)
    heights[i] = 0;

  float persistance = 0.5f;
  float amplitude = 1.0f;
  float amplitudeAcc = 0.0f;  // accumulated amplitude used to normalize output
  int octaves = 8;
  for(int o = 0; o < octaves; o++) {
    int pitch = SCREEN_WIDTH >> o == 0 ? 1 : SCREEN_WIDTH >> o;
    for(int x = 0; x < SCREEN_WIDTH; x++) {
      int sample1 = (x / pitch ) * pitch; 
      int sample2 = (sample1 + pitch) % SCREEN_WIDTH;
      float blend = (float)(x - sample1) / (float)pitch;
      float sample = (1.0f - blend) * baseNoise[sample1] + blend * baseNoise[sample2];
      heights[x] += sample * amplitude;
    }
    amplitudeAcc += amplitude;
    amplitude *= persistance;
  }

  // normalize heights
  for(int i = 0; i < SCREEN_WIDTH; i++) {
    heights[i] /= amplitudeAcc;
  }
}

void clear(uint32_t* pixels) {
  for(int i = 0; i < SCREEN_HEIGHT * SCREEN_WIDTH; i++)
    pixels[i] = 0;
} 

int main() {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
    exit(1);
  }

  float *baseNoise = malloc(SCREEN_WIDTH * sizeof(float));
  float *heights = malloc(SCREEN_WIDTH * sizeof(float));

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

	int running = 1;  
  while(running) {

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT)
            running = 0;
    }

    const Uint8* keyboard = SDL_GetKeyboardState(NULL);

  	if (keyboard[SDL_SCANCODE_Q]) running = 0;
  	if (keyboard[SDL_SCANCODE_SPACE]) randomFill(baseNoise);
  	if (keyboard[SDL_SCANCODE_N]) fractalNoise1D(heights, baseNoise);

  	clear(pixels);

  	for(int y = 0; y < SCREEN_HEIGHT; y ++) {
  	  for(int x = 0; x < SCREEN_WIDTH; x++) {
  	    int colHeight = heights[x] * SCREEN_HEIGHT;
  	    if(y >= SCREEN_HEIGHT - colHeight) {
  	      pixels[y * SCREEN_WIDTH + x] = 0x00ff00ff;
  	    }
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
  free(baseNoise);
  free(heights);
  return 0;
}
