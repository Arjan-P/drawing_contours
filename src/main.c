#include <SDL2/SDL.h>
#include <SDL2/SDL_scancode.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

const int SCREEN_WIDTH = 600;
const int SCREEN_HEIGHT = 500;

void randomFill(float *base, int size) {
  for (int i = 0; i < size; i++)
    base[i] = (float)rand() / (float)RAND_MAX;
}

void fractalNoise1D(float *heights1D, float *baseNoise1D) {
  for (int i = 0; i < SCREEN_WIDTH; i++)
    heights1D[i] = 0;

  float persistance = 0.5f;
  float amplitude = 1.0f;
  float amplitudeAcc = 0.0f; // accumulated amplitude used to normalize output
  int octaves = 5;
  for (int o = 0; o < octaves; o++) {
    int pitch = SCREEN_WIDTH >> o;
    if (pitch < 1) pitch = 1;
    for (int x = 0; x < SCREEN_WIDTH; x++) {
      int sample1 = (x / pitch) * pitch;
      int sample2 = (sample1 + pitch) % SCREEN_WIDTH;
      float blend = (float)(x - sample1) / (float)pitch;
      float sample =
          (1.0f - blend) * baseNoise1D[sample1] + blend * baseNoise1D[sample2];
      heights1D[x] += sample * amplitude;
    }
    amplitudeAcc += amplitude;
    amplitude *= persistance;
  }

  // normalize heights
  for (int i = 0; i < SCREEN_WIDTH; i++) {
    heights1D[i] /= amplitudeAcc;
  }
}

void fractalNoise2D(float *heights2D, float *baseNoise2D) {
  for (int i = 0; i < SCREEN_HEIGHT * SCREEN_WIDTH; i++)
    heights2D[i] = 0;

  float persistance = 0.5f;
  float amplitude = 1.0f;
  float amplitudeAcc = 0.0f; // accumulated amplitude used to normalize output
  int octaves = 8;
  for (int o = 0; o < octaves; o++) {
    int pitch = SCREEN_WIDTH >> o;
    if (pitch < 0) pitch = 1;

    for (int y = 0; y < SCREEN_HEIGHT; y++) {
      for (int x = 0; x < SCREEN_WIDTH; x++) {
        int sampleX1 = (x / pitch) * pitch;
        int sampleY1 = (y / pitch) * pitch;
        int sampleX2 = (sampleX1 + pitch) % SCREEN_WIDTH;
        int sampleY2 = (sampleY1 + pitch) % SCREEN_HEIGHT;

        float blendX = (float)(x - sampleX1) / (float)pitch;
        float blendY = (float)(y - sampleY1) / (float)pitch;
        // bilinear interpolation
        // interpolate horizontally between top two corners
        float top = (1.0f - blendX) * baseNoise2D[sampleY1 * SCREEN_WIDTH + sampleX1] + blendX * baseNoise2D[sampleY1 * SCREEN_WIDTH + sampleX2];
        // interpolate horizontally between bottom two corners
        float bottom = (1.0f - blendX) * baseNoise2D[sampleY2 * SCREEN_WIDTH + sampleX1] + blendX * baseNoise2D[sampleY2 * SCREEN_WIDTH + sampleX2];
        // interpolate vertically
        float sample = (1.0f - blendY) * top + blendY * bottom;
        
        heights2D[y * SCREEN_WIDTH + x] += sample * amplitude;
      }
    }

    amplitudeAcc += amplitude;
    amplitude *= persistance;
  }

  for (int y = 0; y < SCREEN_HEIGHT; y++) {
    for (int x = 0; x < SCREEN_WIDTH; x++) {
      heights2D[y * SCREEN_WIDTH + x] /= amplitudeAcc;
    }
  }
}

void clear(uint32_t *pixels) {
  for (int i = 0; i < SCREEN_HEIGHT * SCREEN_WIDTH; i++)
    pixels[i] = 0;
}

int main() {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
    exit(1);
  }

  float *baseNoise1D = malloc(SCREEN_WIDTH * sizeof(float));
  float *heights1D = malloc(SCREEN_WIDTH * sizeof(float));
  float *baseNoise2D = malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(float));
  float *heights2D = malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(float));

  SDL_Window *window = SDL_CreateWindow("Window", SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                                        SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                           SDL_TEXTUREACCESS_STREAMING,
                                           SCREEN_WIDTH, SCREEN_HEIGHT);

  uint32_t pixels[SCREEN_WIDTH * SCREEN_HEIGHT];

  int running = 1;
  while (running) {

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT)
        running = 0;
    }

    const Uint8 *keyboard = SDL_GetKeyboardState(NULL);

    if (keyboard[SDL_SCANCODE_Q])
      running = 0;
    if (keyboard[SDL_SCANCODE_SPACE]) {
      randomFill(baseNoise1D, SCREEN_WIDTH);
      randomFill(baseNoise2D, SCREEN_WIDTH * SCREEN_HEIGHT);
    }
    if (keyboard[SDL_SCANCODE_N])
      fractalNoise1D(heights1D, baseNoise1D);
    if (keyboard[SDL_SCANCODE_M])
      fractalNoise2D(heights2D, baseNoise2D);

    clear(pixels);

    // draw 1D fractal noise
    // for(int y = 0; y < SCREEN_HEIGHT; y ++) {
    //   for(int x = 0; x < SCREEN_WIDTH; x++) {
    //     int colHeight = heights1D[x] * SCREEN_HEIGHT;
    //     if(y >= SCREEN_HEIGHT - colHeight) {
    //       pixels[y * SCREEN_WIDTH + x] = 0x00ff00ff;
    //     }
    //   }
    // }
    //
    // draw 1D base noise
    // for(int y = 0; y < SCREEN_HEIGHT; y ++) {
    //   for(int x = 0; x < SCREEN_WIDTH; x++) {
    //     int colHeight = baseNoise1D[x] * SCREEN_HEIGHT;
    //     if(y >= SCREEN_HEIGHT - colHeight) {
    //       pixels[y * SCREEN_WIDTH + x] = 0x00ff00ff;
    //     }
    //   }
    // }

    // draw 2D base noise
    // for (int y = 0; y < SCREEN_HEIGHT; y++) {
    //   for (int x = 0; x < SCREEN_WIDTH; x++) {
    //     float height = baseNoise2D[y * SCREEN_WIDTH + x];
    //     uint8_t r = 0xff * height;
    //     uint8_t g = 0xff * height;
    //     uint8_t b = 0xff * height;
    //     uint8_t a = 0xff;
    //     uint32_t c = (r << 24) | (g << 16) | (b << 8) | (a << 0);
    //     pixels[y * SCREEN_WIDTH + x] = c;
    //   }
    // }

    // draw 2D fractal noise
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
      for (int x = 0; x < SCREEN_WIDTH; x++) {
        float height = heights2D[y * SCREEN_WIDTH + x];
        uint8_t r = 0xff * height;
        uint8_t g = 0xff * height;
        uint8_t b = 0xff * height;
        uint8_t a = 0xff;
        uint32_t c = (r << 24) | (g << 16) | (b << 8) | (a << 0);
        pixels[y * SCREEN_WIDTH + x] = c;
      }
    }
    

    SDL_UpdateTexture(texture, NULL, pixels, SCREEN_WIDTH * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
  }

  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  free(baseNoise1D);
  free(heights1D);
  free(baseNoise2D);
  free(heights2D);
  return 0;
}
