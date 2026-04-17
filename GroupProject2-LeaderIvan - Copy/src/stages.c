#include "stages.h"
#include "game.h"
#include "raylib.h"
#include "snake.h"

#include "raymath.h"

static bool IsNearWall(GameData *game, int x, int y) {
  for (int dx = -1; dx <= 1; dx++) {
    for (int dy = -1; dy <= 1; dy++) {
      int nx = x + dx;
      int ny = y + dy;
      if (nx >= 0 && nx < GRID_COUNT_X && ny >= 0 && ny < GRID_COUNT_Y) {
        if (game->map[nx][ny] == TILE_WALL)
          return true;
      }
    }
  }
  return false;
}

void LoadStage(GameData *game, int level) {
  game->currentLevel = level;

  // Clear map
  for (int x = 0; x < GRID_COUNT_X; x++) {
    for (int y = 0; y < GRID_COUNT_Y; y++) {
      game->map[x][y] = TILE_EMPTY;
    }
  }

  if (level == 2) {
    // Corners Box layout
    int size = 7;
    for (int i = 0; i < size; i++) {
      game->map[i][0] = TILE_WALL;
      game->map[0][i] = TILE_WALL;
      game->map[GRID_COUNT_X - 1 - i][0] = TILE_WALL;
      game->map[GRID_COUNT_X - 1][i] = TILE_WALL;
      game->map[i][GRID_COUNT_Y - 1] = TILE_WALL;
      game->map[0][GRID_COUNT_Y - 1 - i] = TILE_WALL;
      game->map[GRID_COUNT_X - 1 - i][GRID_COUNT_Y - 1] = TILE_WALL;
      game->map[GRID_COUNT_X - 1][GRID_COUNT_Y - 1 - i] = TILE_WALL;
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
        bool isBorder =
            (x < 2 || x >= GRID_COUNT_X - 2 || y < 2 || y >= GRID_COUNT_Y - 2);
        int mid = GRID_COUNT_X / 2;
        bool isGateX = (x >= mid - 2 && x <= mid + 1);
        bool isGateY = (y >= mid - 2 && y <= mid + 1);
        if (isBorder) {
          if ((x < 2 || x >= GRID_COUNT_X - 2) && isGateY)
            continue;
          if ((y < 2 || y >= GRID_COUNT_Y - 2) && isGateX)
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

      // 5x5 Spawn Safe Zone check
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

      // 5x5 Spawn Safe Zone check
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

    // 5x5 Spawn Safe Zone check
    if (rx >= 13 && rx <= 17 && ry >= 13 && ry <= 17)
      continue;

    // Rule: Portals must be at least 1 block away from any wall
    if (IsNearWall(game, rx, ry))
      continue;

    if (game->map[rx][ry] == TILE_EMPTY) {
      Vector2 newPos = {(float)rx, (float)ry};

      // Rule: Portal 2 must be at least 7 blocks away from Portal 1
      if (p == 1) {
        if (Vector2Distance(newPos, game->portals[0]) < 12.000001f)
          continue;
      }

      game->map[rx][ry] = TILE_PORTAL;
      game->portals[p] = newPos;
      p++;
    }
  }
}
