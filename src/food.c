#include "food.h"
#include "game.h"

void InitFood(Food *food, Snake *snake, GameData *game) { SpawnFood(food, snake, game); }

void SpawnFood(Food *food, Snake *snake, GameData *game) {
  // Stage 1: Random Sampling
  for (int attempts = 0; attempts < 64; attempts++) {
    int rx = GetRandomValue(0, GRID_COUNT_X - 1);
    int ry = GetRandomValue(0, GRID_COUNT_Y - 1);

    if (game->map[rx][ry] != TILE_EMPTY)
      continue;

    bool inside = false;
    for (int i = 0; i < snake->length; i++) {
      if ((int)snake->body[i].x == rx && (int)snake->body[i].y == ry) {
        inside = true;
        break;
      }
    }

    if (!inside) {
      food->position = (Vector2){(float)rx, (float)ry};
      return;
    }
  }

  // Stage 2: Exhaustive Fallback
  Vector2 emptyCells[GRID_COUNT_X * GRID_COUNT_Y];
  int emptyCount = 0;

  for (int x = 0; x < GRID_COUNT_X; x++) {
    for (int y = 0; y < GRID_COUNT_Y; y++) {
      if (game->map[x][y] != TILE_EMPTY)
        continue;

      bool inside = false;
      for (int i = 0; i < snake->length; i++) {
        if ((int)snake->body[i].x == x && (int)snake->body[i].y == y) {
          inside = true;
          break;
        }
      }
      if (!inside) {
        emptyCells[emptyCount++] = (Vector2){(float)x, (float)y};
      }
    }
  }

  if (emptyCount > 0) {
    int randomIndex = GetRandomValue(0, emptyCount - 1);
    food->position = emptyCells[randomIndex];
  } else {
    food->position = (Vector2){-1.0f, -1.0f};
  }
}

void DrawFood(Food* food, Texture2D fruitTex) {
    Vector2 center = { 
        food->position.x * GRID_SIZE + (GRID_SIZE / 2.0f), 
        food->position.y * GRID_SIZE + (GRID_SIZE / 2.0f) 
    };
    
    // Radius slightly smaller than the grid cell
    float radius = GRID_SIZE / 2.0f * 0.8f;
    
    if (fruitTex.id != 0) {
        // Draw the user provided fruit.png
        float diam = radius * 2.0f;
        float scale = diam / (float)fruitTex.width;
        Rectangle source = {0, 0, (float)fruitTex.width, (float)fruitTex.height};
        Rectangle dest = {center.x, center.y, diam, diam};
        Vector2 origin = {diam / 2.0f, diam / 2.0f};
        DrawTexturePro(fruitTex, source, dest, origin, 0.0f, WHITE);
    } else {
        // Fallback orange circle
        DrawCircle(center.x, center.y, radius, ORANGE);
        DrawCircle(center.x - radius*0.3f, center.y - radius*0.3f, radius*0.2f, WHITE);
    }
}
