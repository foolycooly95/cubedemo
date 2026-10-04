#include "types.h"
#include <SDL3/SDL.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define WIDTH 384
#define HEIGHT 224

static uint32_t pixelbuffer[HEIGHT][WIDTH];
static uint32_t framebuffer[HEIGHT * 2][WIDTH];

Scene scene = SCENE_STATIC;
CubeRotation cube_rotation = CUBE_ROTATE_RIGHT;

double angleX = 0.0;
double angleY = 0.0;

vec3 cube[8] = {{0.25, 0.25, 0.25},    {-0.25, 0.25, 0.25},
                {-0.25, -0.25, 0.25},  {0.25, -0.25, 0.25},
                {0.25, 0.25, -0.25},   {-0.25, 0.25, -0.25},
                {-0.25, -0.25, -0.25}, {0.25, -0.25, -0.25}};

/* Clear the entire pixelbuffer with a unique color */
void clear(uint32_t color) {
  for (uint32_t i = 0; i < HEIGHT; i++) {
    for (uint32_t j = 0; j < WIDTH; j++) {
      pixelbuffer[i][j] = color;
    }
  }
}

void effectstatic(void) {
  int x;

  for (uint32_t i = 0; i < HEIGHT * 2; i++) {
    for (uint32_t j = 0; j < WIDTH; j++) {
      x = rand() % 2;
      if (x) {
        framebuffer[i][j] = 0xFFFFFF;
      } else {
        framebuffer[i][j] = 0x000000;
      }
    }
  }
}

void drawpoint(vec2 p, uint32_t size, uint32_t color) {
  uint32_t x = (p.x + 1) / 2 * WIDTH;
  uint32_t y = (1 - (p.y + 1) / 2) * HEIGHT;
  uint32_t i, j;

  for (i = x - size; i < x + size + 1; i++) {
    for (j = y - size; j < y + size + 1; j++) {
      if (i < 0 || i >= WIDTH || j < 0 || j >= HEIGHT) {
        continue;
      }

      pixelbuffer[j][i] = color;
    }
  }
}

void drawline(vec2 a, vec2 b, uint32_t color) {
  int x0 = (a.x + 1) / 2 * WIDTH;
  int y0 = (1 - (a.y + 1) / 2) * HEIGHT;

  int x1 = (b.x + 1) / 2 * WIDTH;
  int y1 = (1 - (b.y + 1) / 2) * HEIGHT;

  int dx = abs(x1 - x0);
  int dy = abs(y1 - y0);

  int sx = x0 < x1 ? 1 : -1;
  int sy = y0 < y1 ? 1 : -1;

  int err = dx - dy;

  while (1) {
    if (x0 >= 0 && x0 < WIDTH && y0 >= 0 && y0 < HEIGHT) {
      pixelbuffer[y0][x0] = color;
    }

    if (x0 == x1 && y0 == y1) {
      break;
    }

    int e2 = 2 * err;

    if (e2 > -dy) {
      err -= dy;
      x0 += sx;
    }

    if (e2 < dx) {
      err += dx;
      y0 += sy;
    }
  }
}

vec2 project(vec3 p) {
  vec2 out;

  out.x = p.x / p.z;
  out.y = p.y / p.z;

  return out;
}

vec3 translate(vec3 p, uint32_t f) {
  vec3 out;

  out.x = p.x;
  out.y = p.y;
  out.z = p.z + f;

  return out;
}

vec3 rotate(vec3 p, double x, double y) {
  double cx = cos(x);
  double sx = sin(x);
  double cy = cos(y);
  double sy = sin(y);

  vec3 out;

  // Rotate around X
  out.x = p.x;
  out.y = p.y * cx - p.z * sx;
  out.z = p.y * sx + p.z * cx;

  // Rotate around Y
  double x2 = out.x;
  double z2 = out.z;

  out.x = x2 * cy - z2 * sy;
  out.z = x2 * sy + z2 * cy;

  return out;
}

int setupSDL(SDL_Window **window, SDL_Renderer **renderer,
             SDL_Texture **texture) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
    return EXIT_FAILURE;
  }

  *window = SDL_CreateWindow("Cube Demo", HEIGHT * 2 * 1.333, HEIGHT * 2, 0);

  if (*window == NULL) {
    fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
    SDL_Quit();
    return EXIT_FAILURE;
  }

  *renderer = SDL_CreateRenderer(*window, NULL);

  if (*renderer == NULL) {
    fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());

    SDL_DestroyWindow(*window);
    SDL_Quit();

    return EXIT_FAILURE;
  }

  *texture = SDL_CreateTexture(*renderer, SDL_PIXELFORMAT_XRGB8888,
                               SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT * 2);

  if (*texture == NULL) {
    fprintf(stderr, "SDL_CreateTexture failed: %s\n", SDL_GetError());

    SDL_DestroyRenderer(*renderer);
    SDL_DestroyWindow(*window);
    SDL_Quit();

    return EXIT_FAILURE;
  }

  SDL_SetTextureScaleMode(*texture, SDL_SCALEMODE_NEAREST);

  return 0;
}

void scenedisplay(void) {
  switch (scene) {
  case SCENE_STATIC:
    effectstatic();
    break;

  case SCENE_CUBE:
    for (uint32_t i = 0; i < 5; i += 4) {
      for (uint32_t j = 0; j < 3; j++) {
        drawline(project(translate(rotate(cube[i + j], angleX, angleY), 1)),
                 project(translate(rotate(cube[i + j + 1], angleX, angleY), 1)),
                 0x00FF00);
      }

      drawline(project(translate(rotate(cube[i + 3], angleX, angleY), 1)),
               project(translate(rotate(cube[i + 0], angleX, angleY), 1)),
               0xF00FF00);
    }

    for (uint32_t k = 0; k < 4; k++) {
      drawline(project(translate(rotate(cube[k], angleX, angleY), 1)),
               project(translate(rotate(cube[k + 4], angleX, angleY), 1)),
               0x00FF00);
    }

    break;
  }
}

/* Main function */
int main(void) {
  SDL_Window *window = NULL;
  SDL_Renderer *renderer = NULL;
  SDL_Texture *texture = NULL;
  SDL_Event event;

  uint64_t previous = SDL_GetPerformanceCounter();
  double elapsed_time = 0.0;
  const double target_frame = 1.0 / 60.0;
  uint32_t is_running = 1;

  if (setupSDL(&window, &renderer, &texture)) {
    return EXIT_FAILURE;
  }

  /* Frame loop */
  while (is_running) {
    uint64_t now = SDL_GetPerformanceCounter();

    /* Poll events */
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        is_running = false;
      }
    }

    clear(0x000000);

    scenedisplay();

    if (scene != SCENE_STATIC) {
      /* Copy pixelbuffer to framebuffer */
      for (uint32_t line = 0; line < HEIGHT; line++) {
        for (uint32_t pxl = 0; pxl < WIDTH; pxl++) {
          framebuffer[line * 2][pxl] = pixelbuffer[line][pxl];
          framebuffer[line * 2 + 1][pxl] = 0x000000; // apply scanline effect
        }
      }
    }

    /* Copy contents of the framebuffer to our SDL texture */
    SDL_UpdateTexture(texture, NULL, framebuffer, WIDTH * sizeof(uint32_t));

    /* Render texture and display the renderer */
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    double dt =
        (double)(now - previous) / (double)SDL_GetPerformanceFrequency();
    previous = now;

    /* Cap the framerate to 60 FPS (16 milliseconds each frame) */
    if (dt < target_frame) {
      SDL_Delay((uint32_t)((target_frame - dt) * 1000.0));
    }

    elapsed_time += dt;

    switch (scene) {
    case SCENE_STATIC:
      if (elapsed_time > 2) {
        scene = SCENE_CUBE;
      }

      break;

    case SCENE_CUBE:
      switch (cube_rotation) {
      case CUBE_ROTATE_RIGHT:
        angleY += 1.5 * M_PI * dt;

        if (angleY > M_PI * 5) {
          angleY = M_PI * 5;
          cube_rotation = CUBE_ROTATE_DOWN;
        }

        break;

      case CUBE_ROTATE_DOWN:
        angleX += 1.5 * M_PI * dt;

        if (angleX > M_PI * 5) {
          angleX = M_PI * 5;
          cube_rotation = CUBE_ROTATE_LEFT;
        }

        break;

      case CUBE_ROTATE_LEFT:
        angleY -= 1.5 * M_PI * dt;

        if (angleY < 0) {
          angleY = 0;
          cube_rotation = CUBE_ROTATE_UP;
        }

        break;

      case CUBE_ROTATE_UP:
        angleX -= 1.5 * M_PI * dt;

        if (angleX < 0) {
          angleX = 0;
          cube_rotation = CUBE_ROTATE_RIGHT;
        }

        break;
      }

      break;
    }
  }

  /* Free all the SDL resources we created */
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return EXIT_SUCCESS;
}
