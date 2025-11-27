#ifndef GAMEBOX_H
#define GAMEBOX_H

#include <SDL2/SDL.h>
#include <vector>
#include <string>

struct Platform {
    SDL_Rect rect;
    bool isBreakable;
    bool isBrick;
    bool isHit;
};

struct Enemy {
    float x, y;
    float vx;
    SDL_Rect rect;
    bool active;
    float leftBound;    // Tambahan: batas kiri enemy patrol
    float rightBound;   // Tambahan: batas kanan enemy patrol
};

struct Coin {
    int x, y;
    bool collected;
    float animPhase;
};

struct FloatingText {
    float x, y;
    float vy;
    int value;
    Uint32 spawnTime;
    bool active;
};

// Struct baru untuk level chunk system
struct LevelChunk {
    std::vector<Platform> platforms;
    std::vector<Coin> coins;
    std::vector<Enemy> enemies;
    int startX;         // Posisi X awal chunk
    int width;          // Lebar chunk dalam pixels
};

bool runGameBox(SDL_Renderer* renderer);
extern int currentStage;

#endif
