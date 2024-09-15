#include "math.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_main.h>

#define WIDTH 1000
#define HEIGHT 700
#define CELL 5

typedef struct {
  int x;
  int y;
} vec2;

typedef struct {
  vec2 pos;
  struct {
    bool left;
    bool middle;
    bool right;
  } button;
} Mouse;

Mouse mouse;
uint8_t grid[WIDTH / CELL][HEIGHT / CELL];
uint8_t next_grid[WIDTH / CELL][HEIGHT / CELL];

void calculate_physics();

int main(int argc, char **argv) {
  srand(time(NULL));
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

  SDL_Window *window =
      SDL_CreateWindow("Sandbox", SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);

  bool running = true;
  while (running) {
    for (SDL_Event event; SDL_PollEvent(&event);) {
      switch (event.type) {
      case SDL_QUIT:
        running = false;
        break;
      case SDL_KEYDOWN:
        if (event.key.keysym.scancode == SDL_SCANCODE_Q)
          running = false;
        else if (event.key.keysym.scancode == SDL_SCANCODE_C) {
          memset(grid, 0, sizeof(grid));
          memset(next_grid, 0, sizeof(next_grid));
        } else if (event.key.keysym.scancode == SDL_SCANCODE_R) {
          for (int x = 0; x < WIDTH / CELL; x++) {
            for (int y = 0; y < HEIGHT / CELL; y++) {
              grid[x][y] = rand() % 255 + 1;
            }
          }
        }
      case SDL_MOUSEBUTTONDOWN:
        if (event.button.button == SDL_BUTTON_LEFT)
          mouse.button.left = true;
        if (event.button.button == SDL_BUTTON_RIGHT)
          mouse.button.right = true;
        break;
      case SDL_MOUSEBUTTONUP:
        if (event.button.button == SDL_BUTTON_LEFT)
          mouse.button.left = false;
        if (event.button.button == SDL_BUTTON_RIGHT)
          mouse.button.right = false;
        break;
      }
    }

    SDL_GetMouseState(&mouse.pos.x, &mouse.pos.y);
    mouse.pos.x = clamp(mouse.pos.x, 0, WIDTH - 1);
    mouse.pos.y = clamp(mouse.pos.y, 0, HEIGHT - 1);

    calculate_physics();
    memcpy(grid, next_grid, sizeof(grid));
    memset(next_grid, 0, sizeof(next_grid));

    static uint8_t last_green = 1;
    static bool last_green_direction = true;

    if (mouse.button.left && !grid[mouse.pos.x / CELL][mouse.pos.y / CELL]) {
      grid[mouse.pos.x / CELL][mouse.pos.y / CELL] = last_green;

      last_green += last_green_direction ? 1 : -1;
      if (last_green == 1 || last_green == 255)
        last_green_direction = !last_green_direction;
    } else if (mouse.button.right) {
      grid[mouse.pos.x / CELL][mouse.pos.y / CELL] = 0;
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    for (int x = 0; x < WIDTH / CELL; x++) {
      for (int y = 0; y < HEIGHT / CELL; y++) {
        if (grid[x][y]) {
          SDL_Rect rect = {x * CELL, y * CELL, CELL, CELL};
          SDL_SetRenderDrawColor(renderer, 255, grid[x][y], 0, 255);
          SDL_RenderFillRect(renderer, &rect);
        }
      }
    }

    SDL_RenderPresent(renderer);
    SDL_Delay(1000 / 240);
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}

void calculate_physics() {
  for (int x = 0; x < WIDTH / CELL; x++) {
    for (int y = 0; y < HEIGHT / CELL; y++) {
      if (!grid[x][y])
        continue;

      if (y == HEIGHT / CELL - 1) {
        next_grid[x][y] = grid[x][y];
        continue;
      }

      if (!grid[x][y + 1]) {
        next_grid[x][y + 1] = grid[x][y];
        continue;
      }

      if (x == 0) {
        if (!grid[x + 1][y + 1]) {
          next_grid[x + 1][y + 1] = grid[x][y];
          continue;
        }
      } else if (x == WIDTH / CELL - 1) {
        if (!grid[x - 1][y + 1]) {
          next_grid[x - 1][y + 1] = grid[x][y];
          continue;
        }
      } else {
        int dir = rand() % 2 ? 1 : -1;
        if (!grid[x + dir][y + 1]) {
          next_grid[x + dir][y + 1] = grid[x][y];
          continue;
        } else if (!grid[x - dir][y + 1]) {
          next_grid[x - dir][y + 1] = grid[x][y];
          continue;
        }
      }

      next_grid[x][y] = grid[x][y];
    }
  }
}
