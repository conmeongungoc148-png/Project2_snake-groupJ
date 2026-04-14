#ifndef FOOD_H
#define FOOD_H

#include "raylib.h"
#include "snake.h"

typedef struct Food {
    Vector2 position;
} Food;

void InitFood(Food* food, Snake* snake);
void SpawnFood(Food* food, Snake* snake);
void DrawFood(Food* food, Texture2D fruitTex);

#endif // FOOD_H

