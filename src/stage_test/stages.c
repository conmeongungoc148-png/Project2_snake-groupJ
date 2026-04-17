#include "stages.h"
#include "game.h"
#include "raylib.h"
#include "snake.h"
#include <math.h>

void LoadStage(GameData *game, int level) {
  game->currentLevel = level;

  // Clear map
  for (int x = 0; x < GRID_COUNT_X; x++) {
    for (int y = 0; y < GRID_COUNT_Y; y++) {
      game->map[x][y] = TILE_EMPTY;
    }
  }

  if (level == 2) {
    // Corners Box layout - Offset by 1 to allow portal wrapping
    int size = 7;
    for (int i = 1; i < size + 1; i++) {
      game->map[i][1] = TILE_WALL;
      game->map[1][i] = TILE_WALL;
      game->map[GRID_COUNT_X - 2 - i][1] = TILE_WALL;
      game->map[GRID_COUNT_X - 2][i] = TILE_WALL;
      game->map[i][GRID_COUNT_Y - 2] = TILE_WALL;
      game->map[1][GRID_COUNT_Y - 2 - i] = TILE_WALL;
      game->map[GRID_COUNT_X - 2 - i][GRID_COUNT_Y - 2] = TILE_WALL;
      game->map[GRID_COUNT_X - 2][GRID_COUNT_Y - 2 - i] = TILE_WALL;
    }
  } else if (level == 3) {
    // Four Pillars layout
    int cx = GRID_COUNT_X / 2;
    int cy = GRID_COUNT_Y / 2;
    for (int x = cx - 6; x <= cx - 4; x++)
      for (int y = cy - 5; y <= cy - 3; y++)
        game->map[x][y] = TILE_WALL;
    for (int x = cx + 4; x <= cx + 6; x++)
      for (int y = cy - 5; y <= cy - 3; y++)
        game->map[x][y] = TILE_WALL;
    for (int x = cx - 6; x <= cx - 4; x++)
      for (int y = cy + 3; y <= cy + 5; y++)
        game->map[x][y] = TILE_WALL;
    for (int x = cx + 4; x <= cx + 6; x++)
      for (int y = cy + 3; y <= cy + 5; y++)
        game->map[x][y] = TILE_WALL;
  } else if (level == 4) {
    // THE CASTLE
    for (int x = 0; x < GRID_COUNT_X; x++) {
      for (int y = 0; y < GRID_COUNT_Y; y++) {
        // Portal Gap: Outermost tiles (0 and GRID_COUNT-1) are always empty
        if (x == 0 || x == GRID_COUNT_X - 1 || y == 0 || y == GRID_COUNT_Y - 1)
            continue;

        bool isBorder =
            (x < 3 || x >= GRID_COUNT_X - 3 || y < 3 || y >= GRID_COUNT_Y - 3);
        int mid = GRID_COUNT_X / 2;
        bool isGateX = (x >= mid - 2 && x <= mid + 1);
        bool isGateY = (y >= mid - 2 && y <= mid + 1);
        if (isBorder) {
          if ((x < 3 || x >= GRID_COUNT_X - 3) && isGateY)
            continue;
          if ((y < 3 || y >= GRID_COUNT_Y - 3) && isGateX)
            continue;
          game->map[x][y] = TILE_WALL;
        }
      }
    }
    // Inner Brackets
    int offset = 7, len = 4;
    for (int i = 0; i < len; i++) {
      game->map[offset + i][offset] = TILE_WALL;
      game->map[offset][offset + i] = TILE_WALL;
      game->map[GRID_COUNT_X - 1 - offset - i][offset] = TILE_WALL;
      game->map[GRID_COUNT_X - 1 - offset][offset + i] = TILE_WALL;
      game->map[offset + i][GRID_COUNT_Y - 1 - offset] = TILE_WALL;
      game->map[offset][GRID_COUNT_Y - 1 - offset - i] = TILE_WALL;
      game->map[GRID_COUNT_X - 1 - offset - i][GRID_COUNT_Y - 1 - offset] =
          TILE_WALL;
      game->map[GRID_COUNT_X - 1 - offset][GRID_COUNT_Y - 1 - offset - i] =
          TILE_WALL;
    }
  }
}

void LoadChaosStage(GameData *game, int level) {
  // Clear map
  for (int x = 0; x < GRID_COUNT_X; x++) {
    for (int y = 0; y < GRID_COUNT_Y; y++) {
      game->map[x][y] = TILE_EMPTY;
    }
  }

  if (level == 1) {
    int wallCount = 30;
    while (wallCount > 0) {
      int rx = GetRandomValue(2, GRID_COUNT_X - 3);
      int ry = GetRandomValue(2, GRID_COUNT_Y - 3);

      // 5x5 Safe Zone Check (Center 15,15)
      if (rx >= 13 && rx <= 17 && ry >= 13 && ry <= 17)
        continue;

      if (game->map[rx][ry] == TILE_EMPTY) {
        game->map[rx][ry] = TILE_WALL;
        wallCount--;
      }
    }
  } else if (level == 2) {
    LoadStage(game, 2);
  } else if (level == 3) {
    LoadStage(game, 3);
  } else if (level == 4) {
    int wallCount = 60;
    while (wallCount > 0) {
      int rx = GetRandomValue(3, GRID_COUNT_X - 4);
      int ry = GetRandomValue(3, GRID_COUNT_Y - 4);

      // 5x5 Safe Zone Check (Center 15,15)
      if (rx >= 13 && rx <= 17 && ry >= 13 && ry <= 17)
        continue;

      if (game->map[rx][ry] == TILE_EMPTY) {
        game->map[rx][ry] = TILE_WALL;
        wallCount--;
      }
    }
  } else if (level == 5) {
    LoadStage(game, 4); // The Castle
  }

  // Portal Pair (Always refresh)
  game->portals[0] = (Vector2){-1, -1};
  game->portals[1] = (Vector2){-1, -1};
  int p = 0;
  while (p < 2) {
    int rx = GetRandomValue(3, GRID_COUNT_X - 4);
    int ry = GetRandomValue(3, GRID_COUNT_Y - 4);

    // 5x5 Safe Zone Check for Portals
    if (rx >= 13 && rx <= 17 && ry >= 13 && ry <= 17)
      continue;

    if (game->map[rx][ry] == TILE_EMPTY) {
      // Ensure portals are at least 10 blocks apart (Manhattan distance)
      if (p == 1) {
        float dist = fabsf(game->portals[0].x - (float)rx) +
                     fabsf(game->portals[0].y - (float)ry);
        if (dist < 10.0f)
          continue;
      }

      game->map[rx][ry] = TILE_PORTAL;
      game->portals[p] = (Vector2){(float)rx, (float)ry};
      p++;
    }
  }
}
