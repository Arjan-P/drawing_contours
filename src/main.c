#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

const int SCREEN_WIDTH = 600;
const int SCREEN_HEIGHT = 600;
const int CELL_SIZE = 2;
const int HEIGHT_MAP_HEIGHT = SCREEN_HEIGHT / CELL_SIZE;
const int HEIGHT_MAP_WIDTH = SCREEN_WIDTH / CELL_SIZE;

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
  for (int i = 0; i < HEIGHT_MAP_HEIGHT * HEIGHT_MAP_WIDTH; i++)
    heights2D[i] = 0;

  float persistance = 0.5f;
  float amplitude = 1.0f;
  float amplitudeAcc = 0.0f; // accumulated amplitude used to normalize output
  int octaves = 5;
  for (int o = 0; o < octaves; o++) {
    int pitchX = (HEIGHT_MAP_WIDTH)  / (1 << o);
    int pitchY = (HEIGHT_MAP_HEIGHT) / (1 << o);

    if (pitchX < 1) pitchX = 1;
    if (pitchY < 1) pitchY = 1;


    for (int y = 0; y < HEIGHT_MAP_HEIGHT; y++) {
      for (int x = 0; x < HEIGHT_MAP_WIDTH; x++) {
        int sampleX1 = (x / pitchX) * pitchX;
        int sampleY1 = (y / pitchY) * pitchY;
        int sampleX2 = (sampleX1 + pitchX) % HEIGHT_MAP_WIDTH;
        int sampleY2 = (sampleY1 + pitchY) % HEIGHT_MAP_HEIGHT;

        float blendX = (float)(x - sampleX1) / (float)pitchX;
        float blendY = (float)(y - sampleY1) / (float)pitchY;
        // to smooth interpolation
        blendX = blendX * blendX * (3 - 2 * blendX);
        blendY = blendY * blendY * (3 - 2 * blendY);

        // bilinear interpolation
        // interpolate horizontally between top two corners
        float top = (1.0f - blendX) * baseNoise2D[sampleY1 * HEIGHT_MAP_WIDTH + sampleX1] + blendX * baseNoise2D[sampleY1 * HEIGHT_MAP_WIDTH + sampleX2];
        // interpolate horizontally between bottom two corners
        float bottom = (1.0f - blendX) * baseNoise2D[sampleY2 * HEIGHT_MAP_WIDTH + sampleX1] + blendX * baseNoise2D[sampleY2 * HEIGHT_MAP_WIDTH + sampleX2];
        // interpolate vertically
        float sample = (1.0f - blendY) * top + blendY * bottom;
        
        heights2D[y * HEIGHT_MAP_WIDTH + x] += sample * amplitude;
      }
    }

    amplitudeAcc += amplitude;
    amplitude *= persistance;
  }

  for(int i = 0; i < HEIGHT_MAP_HEIGHT * HEIGHT_MAP_WIDTH; i++)
      heights2D[i] /= amplitudeAcc;
}

void clear(uint32_t *pixels) {
  for (int i = 0; i < SCREEN_HEIGHT * SCREEN_WIDTH; i++)
    pixels[i] = 0;
}

void drawLine(int x0, int y0, int x1, int y1, uint32_t* pixels)
{
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);

    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;

    int err = dx - dy;

    while (1)
    {
        // Draw pixel if inside bounds
        if (x0 >= 0 && x0 < SCREEN_WIDTH &&
            y0 >= 0 && y0 < SCREEN_HEIGHT)
        {
            pixels[y0 * SCREEN_WIDTH + x0] = 0xFF0000FF;
        }

        if (x0 == x1 && y0 == y1)
            break;

        int e2 = 2 * err;

        if (e2 > -dy)
        {
            err -= dy;
            x0 += sx;
        }

        if (e2 < dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}


int main() {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
    exit(1);
  }

  float *baseNoise1D = malloc(SCREEN_WIDTH * sizeof(float));
  float *heights1D = malloc(SCREEN_WIDTH * sizeof(float));
  float *baseNoise2D = malloc(HEIGHT_MAP_WIDTH * HEIGHT_MAP_HEIGHT * sizeof(float));
  float *heights2D = malloc(HEIGHT_MAP_WIDTH * HEIGHT_MAP_HEIGHT * sizeof(float));

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
      //randomFill(baseNoise1D, SCREEN_WIDTH);
      randomFill(baseNoise2D, HEIGHT_MAP_HEIGHT * HEIGHT_MAP_WIDTH);
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
    for(int y = 0; y < SCREEN_HEIGHT; y++) {
      for(int x = 0; x < SCREEN_WIDTH; x++) {
        int heightX = x / CELL_SIZE;
        int heightY = y / CELL_SIZE;
        float height = heights2D[heightY * HEIGHT_MAP_WIDTH + heightX];
        uint32_t c = (height > 0.4f) ? 0x00fff0ff : 0x000000ff;
        pixels[y * SCREEN_WIDTH + x] = c;
      }
    }

    // draw contour lines
    for (int y = 0; y < HEIGHT_MAP_HEIGHT - 1; y++) {
      for (int x = 0; x < HEIGHT_MAP_WIDTH - 1; x++) {
        int screenX = x * CELL_SIZE;
        int screenY = y * CELL_SIZE;
        float threshold = 0.4f;
        float tl = heights2D[(y + 0) * (HEIGHT_MAP_WIDTH) + (x + 0)];
        float tr = heights2D[(y + 0) * (HEIGHT_MAP_WIDTH) + (x + 1)];
        float bl = heights2D[(y + 1) * (HEIGHT_MAP_WIDTH) + (x + 0)];
        float br = heights2D[(y + 1) * (HEIGHT_MAP_WIDTH) + (x + 1)];
        uint8_t i = 0x00;
        if(tl > threshold) i |= 1 << 3;
        if(tr > threshold) i |= 1 << 2;
        if(br > threshold) i |= 1 << 1;
        if(bl > threshold) i |= 1 << 0;

        int screenX1 = screenX + (CELL_SIZE / 2);
        int screenY1 = screenY + (CELL_SIZE / 2); 
        int screenX2 = screenX + (CELL_SIZE); 
        int screenY2 = screenY + (CELL_SIZE); 
        // draw contour lines using midpoint
        switch (i) {
          case(0):
            // 0----0
            // |    |
            // |    |
            // 0----0
            break;
          case(1):
            // 0----0
            // |    |
            // |\   |
            // 1----0
            drawLine(screenX, screenY1, screenX1, screenY2, pixels);
            break;
          case(2):
            // 0----0
            // |    |
            // |   /|
            // 0----1
            drawLine(screenX1, screenY2, screenX2, screenY1, pixels);
            break;
          case(3):
            // 0----0
            // |____|
            // |    |
            // 1----1
            drawLine(screenX, screenY1, screenX2, screenY1, pixels);
            break;
          case(4):
            // 0----1
            // |   \|
            // |    |
            // 0----0
            drawLine(screenX1, screenY, screenX2, screenY1, pixels);
            break;
          case(5):
            // 0----1
            // |/   |
            // |   /|
            // 1----0
            drawLine(screenX, screenY1, screenX1, screenY, pixels);
            drawLine(screenX1, screenY2, screenX2, screenY1, pixels);
            break;
          case(6):
            // 0----1
            // | |  |
            // | |  |
            // 0----1
            drawLine(screenX1, screenY, screenX1, screenY2, pixels);
            break;
          case(7):
            // 0----1
            // |/   |
            // |    |
            // 1----1
            drawLine(screenX, screenY1, screenX1, screenY, pixels);
            break;
          case(8):
            // 1----0
            // |/   |
            // |    |
            // 0----0
            drawLine(screenX, screenY1, screenX1, screenY, pixels);
            break;
          case(9):
            // 1----0
            // | |  |
            // | |  |
            // 1----0
            drawLine(screenX1, screenY, screenX1, screenY2, pixels);
            break;
          case(10):
            // 1----0
            // |   \|
            // |\   |
            // 0----1
            drawLine(screenX1, screenY, screenX2, screenY1, pixels);
            drawLine(screenX, screenY1, screenX1, screenY2, pixels);
            break;
          case(11):
            // 1----0
            // |   \|
            // |    |
            // 1----1
            drawLine(screenX1, screenY, screenX2, screenY1, pixels);
            break;
          case(12):
            // 1----1
            // |____|
            // |    |
            // 0----0
            drawLine(screenX, screenY1, screenX2, screenY1, pixels);
            break;
          case(13):
            // 1----1
            // |    |
            // |   /|
            // 1----0
            drawLine(screenX1, screenY2, screenX2, screenY1, pixels);
            break;
          case(14):
            // 1----1
            // |    |
            // |\   |
            // 0----1
            drawLine(screenX, screenY1, screenX1, screenY2, pixels);
            break;
          case(15):
            // 1----1
            // |    |
            // |    |
            // 1----1
            break;
        } 
        //pixels[y * SCREEN_WIDTH + x] = c;
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
