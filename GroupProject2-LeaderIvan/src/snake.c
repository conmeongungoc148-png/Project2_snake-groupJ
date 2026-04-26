#include "snake.h"
#include "game.h"
#include <math.h>

void InitSnake(Snake *snake) {
  snake->length = 3;
  
  int startX = GRID_COUNT_X / 2;
  int startY = GRID_COUNT_Y / 2;

  
  for (int i = 0; i < snake->length; i++) {
    snake->body[i] = (Vector2){(float)startX, (float)(startY + i)};
  }

  snake->direction = (Vector2){0, -1}; 
  snake->moveTimer = 0.0f;
  snake->timePerMove = 0.1f; 
  snake->inputQueueLength = 0;
  snake->isAlive = true;
  snake->justAteFood = false;
  snake->invincibilityTimer = 0.0f;
}

void HandleSnakeInput(Snake *snake) {
  Vector2 nextDir = {0};
  bool keyHit = false;

  if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
    nextDir = (Vector2){0, -1};
    keyHit = true;
  } else if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
    nextDir = (Vector2){0, 1};
    keyHit = true;
  } else if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
    nextDir = (Vector2){-1, 0};
    keyHit = true;
  } else if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
    nextDir = (Vector2){1, 0};
    keyHit = true;
  }

  if (keyHit) {
    
    Vector2 currentDir = snake->direction;
    if (snake->inputQueueLength > 0) {
      currentDir = snake->inputQueue[snake->inputQueueLength - 1];
    }

    
    
    
    if ((nextDir.x != 0 && nextDir.x != currentDir.x &&
         nextDir.x != -currentDir.x) ||
        (nextDir.y != 0 && nextDir.y != currentDir.y &&
         nextDir.y != -currentDir.y)) {

      
      if (snake->inputQueueLength < 2) {
        snake->inputQueue[snake->inputQueueLength] = nextDir;
        snake->inputQueueLength++;
      }
    }
  }
}

void UpdateSnakeLogic(Snake *snake, GameData *game) {
  if (!snake->isAlive)
    return;

  
  if (snake->inputQueueLength > 0) {
    snake->direction = snake->inputQueue[0];
    
    snake->inputQueue[0] = snake->inputQueue[1];
    snake->inputQueueLength--;
  }

  
  if (snake->justAteFood) {
    if (snake->length < MAX_SNAKE_LENGTH) {
      snake->length++;
    }
    snake->justAteFood = false;
  }

  
  for (int i = snake->length - 1; i > 0; i--) {
    snake->body[i] = snake->body[i - 1];
  }

  
  snake->body[0].x += snake->direction.x;
  snake->body[0].y += snake->direction.y;

  
  if (snake->body[0].x < 0)
    snake->body[0].x = GRID_COUNT_X - 1;
  else if (snake->body[0].x >= GRID_COUNT_X)
    snake->body[0].x = 0;

  if (snake->body[0].y < 0)
    snake->body[0].y = GRID_COUNT_Y - 1;
  else if (snake->body[0].y >= GRID_COUNT_Y)
    snake->body[0].y = 0;

  
}

bool CheckCollisionWithSelfOrWall(Snake *snake, GameData *game) {
  Vector2 head = snake->body[0];

  
  /*
  if (head.x < 0 || head.x >= GRID_COUNT_X || head.y < 0 ||
      head.y >= GRID_COUNT_Y) {
    snake->isAlive = false;
    return true;
  }
  */

  
  if (game->map[(int)head.x][(int)head.y] == TILE_WALL) {
    if (snake->invincibilityTimer <= 0) {
      snake->isAlive = false;
      return true;
    }
  }

  
  if (snake->invincibilityTimer <= 0) {
    for (int i = 1; i < snake->length; i++) {
      if (head.x == snake->body[i].x && head.y == snake->body[i].y) {
        snake->isAlive = false;
        return true;
      }
    }
  }

  return false;
}

void ReverseSnake(Snake *snake) {
  for (int i = 0; i < snake->length / 2; i++) {
    Vector2 temp = snake->body[i];
    snake->body[i] = snake->body[snake->length - 1 - i];
    snake->body[snake->length - 1 - i] = temp;
  }
  if (snake->length > 1) {
    float dx = snake->body[0].x - snake->body[1].x;
    float dy = snake->body[0].y - snake->body[1].y;

    
    if (dx > 1) dx = -1;
    else if (dx < -1) dx = 1;
    if (dy > 1) dy = -1;
    else if (dy < -1) dy = 1;

    snake->direction.x = dx;
    snake->direction.y = dy;
  }
  snake->inputQueueLength = 0;
}


void DrawTextureCenteredEx(Texture2D tex, Vector2 pos, float rotation,
                           float scale, bool flipX, Color tint) {
  if (tex.id == 0)
    return;
  float targetWidth = tex.width * scale;
  float targetHeight = tex.height * scale;

  Rectangle source = {0, 0, (float)tex.width, (float)tex.height};
  if (flipX)
    source.width = -source.width;

  Rectangle dest = {pos.x, pos.y, targetWidth, targetHeight};
  Vector2 origin = {targetWidth / 2.0f, targetHeight / 2.0f};

  DrawTexturePro(tex, source, dest, origin, rotation, tint);
}

void DrawSnake(Snake *snake, Texture2D headTex, Texture2D bodyTex,
                Texture2D tailTex) {
  float targetSize = (float)GRID_SIZE * 1.20f; 

  
  Color snakeAlphaTint = WHITE;
  if (snake->invincibilityTimer > 0) {
    float pulse = (sinf((float)GetTime() * 20.0f) + 1.0f) * 0.4f + 0.2f;
    snakeAlphaTint = (Color){255, 255, 255, (unsigned char)(255 * pulse)};
  }

  for (int i = snake->length - 1; i >= 0; i--) {
    Vector2 gridPos = snake->body[i];
    Vector2 center = {gridPos.x * GRID_SIZE + (GRID_SIZE / 2.0f),
                      gridPos.y * GRID_SIZE + (GRID_SIZE / 2.0f)};

    

    

    if (i == 0) { 
      if (headTex.id != 0) {
        float rot = 0.0f;
        bool flipH = false;

        
        float pushUpAmount = targetSize * 0.22f;

        
        if (snake->direction.x == 1) {
          rot = 0.0f;
          flipH = false;
          center.y -= pushUpAmount;
        } 
        else if (snake->direction.x == -1) {
          rot = 0.0f;
          flipH = true;
          center.y -= pushUpAmount;
        } 
        else if (snake->direction.y == 1) {
          rot = 90.0f;
          flipH = false;
          center.x += pushUpAmount;
        } 
        else if (snake->direction.y == -1) {
          rot = 270.0f;
          flipH = false;
          center.x -= pushUpAmount;
        } 

        
        float headScale = (targetSize * 2.2f) / (float)headTex.width;
        DrawTextureCenteredEx(headTex, center, rot,
                              headScale, flipH,
                              snakeAlphaTint);
      } else {
        DrawRectangle(gridPos.x * GRID_SIZE, gridPos.y * GRID_SIZE, GRID_SIZE,
                      GRID_SIZE, YELLOW);
      }
    } else if (i == snake->length - 1) { 
      if (tailTex.id != 0) {
        Vector2 diff = {snake->body[i - 1].x - snake->body[i].x,
                        snake->body[i - 1].y - snake->body[i].y};
        
        
        if (diff.x > 1) diff.x = -1;
        else if (diff.x < -1) diff.x = 1;
        if (diff.y > 1) diff.y = -1;
        else if (diff.y < -1) diff.y = 1;

        float rot = 0.0f;
        bool flipH = false;

        float pushUpAmount = targetSize * -0.08f;

        if (diff.x == 1) {
          rot = 0.0f;
          flipH = false;
          center.y -= pushUpAmount;
        } else if (diff.x == -1) {
          rot = 0.0f;
          flipH = true;
          center.y -= pushUpAmount;
        } else if (diff.y == 1) {
          rot = 90.0f;
          flipH = false;
          center.x += pushUpAmount;
        } else if (diff.y == -1) {
          rot = 270.0f;
          flipH = false;
          center.x -= pushUpAmount;
        }

        float tailScale = (targetSize * 2.2f) / (float)tailTex.width;
        DrawTextureCenteredEx(tailTex, center, rot,
                              tailScale, flipH,
                              snakeAlphaTint);
      } else {
        DrawRectangle(gridPos.x * GRID_SIZE, gridPos.y * GRID_SIZE, GRID_SIZE,
                      GRID_SIZE, YELLOW);
      }
    } else { 
      if (bodyTex.id != 0) {
        Vector2 diff = {snake->body[i - 1].x - snake->body[i].x,
                        snake->body[i - 1].y - snake->body[i].y};

        
        if (diff.x > 1) diff.x = -1;
        else if (diff.x < -1) diff.x = 1;
        if (diff.y > 1) diff.y = -1;
        else if (diff.y < -1) diff.y = 1;

        float rot = 0.0f;
        bool flipH = false;

        if (diff.x == 1) {
          rot = 0.0f;
          flipH = false;
        } else if (diff.x == -1) {
          rot = 0.0f;
          flipH = true;
        } else if (diff.y == 1) {
          rot = 90.0f;
          flipH = false;
        } else if (diff.y == -1) {
          rot = 270.0f;
          flipH = false;
        }

        float size = targetSize * 0.85f;
        Rectangle dest = { center.x, center.y, size, size };
        Vector2 origin = { size / 2.0f, size / 2.0f };
        Color bodyColor = (Color){ 51, 51, 51, 255 }; 
        if (snakeAlphaTint.a < 255) {
            bodyColor.a = snakeAlphaTint.a;
        }
        DrawRectanglePro(dest, origin, rot, bodyColor);
      } else {
        float size = targetSize * 0.85f;
        Rectangle dest = { center.x, center.y, size, size };
        Vector2 origin = { size / 2.0f, size / 2.0f };
        Color bodyColor = (Color){ 51, 51, 51, 255 }; 
        if (snakeAlphaTint.a < 255) {
            bodyColor.a = snakeAlphaTint.a;
        }
        DrawRectanglePro(dest, origin, 0.0f, bodyColor);
      }
    }
  }
}
