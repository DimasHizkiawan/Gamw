#include "GameBox.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <cmath>
#include <iostream>
#include <vector>

// Game constants
const float GRAVITY = 1200.0f;
const float JUMP_FORCE = -700.0f;  // Much higher jump
const float MOVE_SPEED = 250.0f;
const int PLAYER_SIZE = 32;

void renderText(SDL_Renderer* renderer, TTF_Font* font, const char* text, int x, int y, SDL_Color color, bool centered) {
    if (!font) return;
    
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
    if (surface) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_Rect rect = {
            centered ? x - surface->w / 2 : x,
            y - surface->h / 2,
            surface->w,
            surface->h
        };
        SDL_RenderCopy(renderer, texture, nullptr, &rect);
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
    }
}

bool runGameBox(SDL_Renderer* renderer)
{
    // Load font for UI
    TTF_Font* gameFont = nullptr;
    const char* font_paths[] = {
        "assets/PressStart2P-Regular.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "C:\\Windows\\Fonts\\arial.ttf"
    };
    
    for (const char* path : font_paths) {
        gameFont = TTF_OpenFont(path, 24);
        if (gameFont) break;
    }
    
    // Get window size
    int windowWidth, windowHeight;
    SDL_GetRendererOutputSize(renderer, &windowWidth, &windowHeight);
    
    // Player state
    float playerX = 100.0f;
    float playerY = 100.0f;
    float velocityX = 0.0f;
    float velocityY = 0.0f;
    bool isOnGround = false;
    bool facingRight = true;
    
    // Game state
    int score = 0;
    int lives = 3;
    bool gameOver = false;
    Uint32 deathTime = 0;
    
    // Animation
    float animPhase = 0.0f;
    
    // Create platforms (Mario-style)
    std::vector<Platform> platforms;
    
    // Ground platforms - FULL GROUND
    int groundY = windowHeight - 80;
    for (int x = 0; x < windowWidth; x += 32) {
        Platform p;
        p.rect = {x, groundY, 32, 80};
        p.isBreakable = false;
        p.isBrick = true;
        platforms.push_back(p);
    }
    
    // Floating platforms
    platforms.push_back({{250, groundY - 150, 128, 20}, false, true});
    platforms.push_back({{450, groundY - 200, 128, 20}, false, true});
    platforms.push_back({{700, groundY - 180, 96, 20}, false, true});
    platforms.push_back({{900, groundY - 250, 128, 20}, false, true});
    
    // Question blocks
    platforms.push_back({{300, groundY - 280, 32, 32}, true, false});
    platforms.push_back({{500, groundY - 330, 32, 32}, true, false});
    platforms.push_back({{800, groundY - 310, 32, 32}, true, false});
    
    // Coins
    std::vector<Coin> coins;
    coins.push_back({350, groundY - 220, false, 0.0f});
    coins.push_back({550, groundY - 280, false, 0.0f});
    coins.push_back({750, groundY - 240, false, 0.0f});
    coins.push_back({950, groundY - 200, false, 0.0f});
    
    // Simple enemies (Goomba-like)
    std::vector<Enemy> enemies;
    Enemy e1 = {400.0f, static_cast<float>(groundY - 32), 50.0f, {0, 0, 28, 28}, true};
    Enemy e2 = {650.0f, static_cast<float>(groundY - 32), -60.0f, {0, 0, 28, 28}, true};
    enemies.push_back(e1);
    enemies.push_back(e2);
    
    SDL_Event event;
    bool running = true;
    Uint32 lastTime = SDL_GetTicks();
    
    std::cout << "Game started! Ground at Y: " << groundY << std::endl;
    
    while (running)
    {
        // Calculate delta time
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;
        
        if (deltaTime > 0.05f) deltaTime = 0.05f; // Cap delta time
        
        // ------- EVENTS -------
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                return false;
                
            if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    return false;
                case SDLK_SPACE:
                case SDLK_UP:
                case SDLK_w:
                    if (isOnGround && !gameOver) {
                        velocityY = JUMP_FORCE;
                        isOnGround = false;
                    }
                    break;
                case SDLK_r:
                    if (gameOver) {
                        // Restart game
                        return true;
                    }
                    break;
                }
            }
        }
        
        if (!gameOver) {
            // ------- INPUT -------
            const Uint8* keystate = SDL_GetKeyboardState(NULL);
            velocityX = 0.0f;
            
            if (keystate[SDL_SCANCODE_LEFT] || keystate[SDL_SCANCODE_A]) {
                velocityX = -MOVE_SPEED;
                facingRight = false;
            }
            if (keystate[SDL_SCANCODE_RIGHT] || keystate[SDL_SCANCODE_D]) {
                velocityX = MOVE_SPEED;
                facingRight = true;
            }
            
            // ------- PHYSICS -------
            // Apply gravity
            velocityY += GRAVITY * deltaTime;
            
            // Limit fall speed
            if (velocityY > 600.0f) velocityY = 600.0f;
            
            // Store old position
            float oldX = playerX;
            float oldY = playerY;
            
            // Update position
            playerX += velocityX * deltaTime;
            playerY += velocityY * deltaTime;
            
            // Screen wrap
            if (playerX < -PLAYER_SIZE) playerX = windowWidth;
            if (playerX > windowWidth) playerX = -PLAYER_SIZE;
            
            // Create player rect
            SDL_Rect playerRect = {
                static_cast<int>(playerX),
                static_cast<int>(playerY),
                PLAYER_SIZE,
                PLAYER_SIZE
            };
            
            // Check ground collision
            isOnGround = false;
            
            for (auto& platform : platforms) {
                // Check if player overlaps with platform
                bool overlapsX = playerX + PLAYER_SIZE > platform.rect.x && 
                                playerX < platform.rect.x + platform.rect.w;
                bool overlapsY = playerY + PLAYER_SIZE > platform.rect.y && 
                                playerY < platform.rect.y + platform.rect.h;
                
                if (overlapsX && overlapsY) {
                    // Landing on top
                    if (oldY + PLAYER_SIZE <= platform.rect.y && velocityY > 0) {
                        playerY = platform.rect.y - PLAYER_SIZE;
                        velocityY = 0;
                        isOnGround = true;
                    }
                    // Hitting from below (question block)
                    else if (oldY >= platform.rect.y + platform.rect.h && velocityY < 0) {
                        playerY = platform.rect.y + platform.rect.h;
                        velocityY = 0;
                        
                        if (platform.isBreakable) {
                            score += 100;
                            std::cout << "Block hit! Score: " << score << std::endl;
                        }
                    }
                    // Side collision
                    else if (velocityY >= 0) {
                        // Push player to side
                        if (oldX + PLAYER_SIZE <= platform.rect.x) {
                            playerX = platform.rect.x - PLAYER_SIZE;
                        } else if (oldX >= platform.rect.x + platform.rect.w) {
                            playerX = platform.rect.x + platform.rect.w;
                        }
                    }
                }
            }
            
            // Update player rect with new position
            playerRect.x = static_cast<int>(playerX);
            playerRect.y = static_cast<int>(playerY);
            
            // Coin collection
            SDL_Rect coinCollect = {playerRect.x + 4, playerRect.y + 4, playerRect.w - 8, playerRect.h - 8};
            for (auto& coin : coins) {
                if (!coin.collected) {
                    SDL_Rect coinRect = {coin.x - 8, coin.y - 8, 16, 16};
                    if (SDL_HasIntersection(&coinCollect, &coinRect)) {
                        coin.collected = true;
                        score += 50;
                        std::cout << "Coin collected! Score: " << score << std::endl;
                    }
                }
            }
            
            // Update enemies
            for (auto& enemy : enemies) {
                if (!enemy.active) continue;
                
                enemy.x += enemy.vx * deltaTime;
                
                // Keep enemies on platforms
                bool enemyOnGround = false;
                for (const auto& platform : platforms) {
                    if (enemy.x + enemy.rect.w > platform.rect.x && 
                        enemy.x < platform.rect.x + platform.rect.w &&
                        enemy.y + enemy.rect.h >= platform.rect.y &&
                        enemy.y + enemy.rect.h <= platform.rect.y + 10) {
                        enemyOnGround = true;
                        break;
                    }
                }
                
                enemy.rect.x = static_cast<int>(enemy.x);
                enemy.rect.y = static_cast<int>(enemy.y);
                
                // Bounce off screen edges
                if (enemy.x < 0 || enemy.x > windowWidth - enemy.rect.w) {
                    enemy.vx = -enemy.vx;
                }
                
                // Enemy collision with player
                if (SDL_HasIntersection(&playerRect, &enemy.rect)) {
                    // Stomp on enemy (player must be falling and above enemy)
                    if (velocityY > 0 && oldY + PLAYER_SIZE <= enemy.rect.y + 10) {
                        enemy.active = false;
                        velocityY = JUMP_FORCE * 0.5f;
                        score += 200;
                        std::cout << "Enemy defeated! Score: " << score << std::endl;
                    }
                    // Hit by enemy
                    else {
                        lives--;
                        std::cout << "Hit! Lives remaining: " << lives << std::endl;
                        
                        if (lives <= 0) {
                            gameOver = true;
                            deathTime = currentTime;
                            std::cout << "Game Over! Final Score: " << score << std::endl;
                        } else {
                            // Respawn
                            playerX = 100.0f;
                            playerY = 100.0f;
                            velocityX = 0.0f;
                            velocityY = 0.0f;
                        }
                    }
                }
            }
            
            // Fall death
            if (playerY > windowHeight + 50) {
                lives--;
                std::cout << "Fell! Lives remaining: " << lives << std::endl;
                
                if (lives <= 0) {
                    gameOver = true;
                    deathTime = currentTime;
                    std::cout << "Game Over! Final Score: " << score << std::endl;
                } else {
                    playerX = 100.0f;
                    playerY = 100.0f;
                    velocityX = 0.0f;
                    velocityY = 0.0f;
                }
            }
            
            // Update animation
            if (velocityX != 0 && isOnGround) {
                animPhase += deltaTime * 10.0f;
            }
            
            for (auto& coin : coins) {
                coin.animPhase += deltaTime * 3.0f;
            }
        }
        
        // ------- RENDER -------
        // Sky background
        SDL_SetRenderDrawColor(renderer, 92, 148, 252, 255);
        SDL_RenderClear(renderer);
        
        // Clouds (simple)
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        for (int i = 0; i < 3; i++) {
            int cx = 200 + i * 350;
            int cy = 80 + i * 30;
            SDL_Rect cloud = {cx, cy, 60, 30};
            SDL_RenderFillRect(renderer, &cloud);
        }
        
        // Platforms
        for (const auto& platform : platforms) {
            if (platform.isBreakable) {
                // Question block (yellow with ?)
                SDL_SetRenderDrawColor(renderer, 243, 168, 59, 255);
                SDL_RenderFillRect(renderer, &platform.rect);
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderDrawRect(renderer, &platform.rect);
                
                // Draw "?"
                SDL_Rect qmark = {platform.rect.x + 12, platform.rect.y + 8, 8, 16};
                SDL_RenderFillRect(renderer, &qmark);
            } else if (platform.isBrick) {
                // Brown brick or green grass on top
                if (platform.rect.y >= groundY - 5) {
                    // Ground - green grass
                    SDL_SetRenderDrawColor(renderer, 123, 192, 67, 255);
                    SDL_Rect grass = {platform.rect.x, platform.rect.y, platform.rect.w, 20};
                    SDL_RenderFillRect(renderer, &grass);
                    
                    // Dirt below
                    SDL_SetRenderDrawColor(renderer, 139, 90, 43, 255);
                    SDL_Rect dirt = {platform.rect.x, platform.rect.y + 20, platform.rect.w, platform.rect.h - 20};
                    SDL_RenderFillRect(renderer, &dirt);
                } else {
                    // Floating platform - brown brick
                    SDL_SetRenderDrawColor(renderer, 184, 111, 80, 255);
                    SDL_RenderFillRect(renderer, &platform.rect);
                }
                
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderDrawRect(renderer, &platform.rect);
            }
        }
        
        // Coins
        for (const auto& coin : coins) {
            if (!coin.collected) {
                float scale = std::abs(std::cos(coin.animPhase));
                int width = static_cast<int>(16 * scale);
                if (width < 4) width = 4;
                
                SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
                SDL_Rect coinRect = {coin.x - width / 2, coin.y - 8, width, 16};
                SDL_RenderFillRect(renderer, &coinRect);
                
                SDL_SetRenderDrawColor(renderer, 200, 160, 0, 255);
                SDL_RenderDrawRect(renderer, &coinRect);
            }
        }
        
        // Enemies
        for (const auto& enemy : enemies) {
            if (!enemy.active) continue;
            
            // Brown goomba
            SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255);
            SDL_RenderFillRect(renderer, &enemy.rect);
            
            // Eyes
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_Rect eye1 = {enemy.rect.x + 6, enemy.rect.y + 8, 6, 6};
            SDL_Rect eye2 = {enemy.rect.x + 16, enemy.rect.y + 8, 6, 6};
            SDL_RenderFillRect(renderer, &eye1);
            SDL_RenderFillRect(renderer, &eye2);
            
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_Rect pupil1 = {enemy.rect.x + 8, enemy.rect.y + 10, 3, 3};
            SDL_Rect pupil2 = {enemy.rect.x + 18, enemy.rect.y + 10, 3, 3};
            SDL_RenderFillRect(renderer, &pupil1);
            SDL_RenderFillRect(renderer, &pupil2);
        }
        
        if (!gameOver) {
            // Player (Mario-like)
            SDL_Rect playerRect = {
                static_cast<int>(playerX),
                static_cast<int>(playerY),
                PLAYER_SIZE,
                PLAYER_SIZE
            };
            
            // Body
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_Rect body = {playerRect.x + 4, playerRect.y + 8, 24, 16};
            SDL_RenderFillRect(renderer, &body);
            
            // Head
            SDL_SetRenderDrawColor(renderer, 255, 200, 150, 255);
            SDL_Rect head = {playerRect.x + 8, playerRect.y, 16, 16};
            SDL_RenderFillRect(renderer, &head);
            
            // Cap
            SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255);
            SDL_Rect cap = {playerRect.x + 6, playerRect.y - 4, 20, 8};
            SDL_RenderFillRect(renderer, &cap);
            
            // Legs (animated when moving)
            SDL_SetRenderDrawColor(renderer, 0, 0, 200, 255);
            if (isOnGround) {
                int legOffset = static_cast<int>(std::sin(animPhase) * 3);
                SDL_Rect leg1 = {playerRect.x + 8 + legOffset, playerRect.y + 24, 6, 8};
                SDL_Rect leg2 = {playerRect.x + 18 - legOffset, playerRect.y + 24, 6, 8};
                SDL_RenderFillRect(renderer, &leg1);
                SDL_RenderFillRect(renderer, &leg2);
            } else {
                SDL_Rect leg = {playerRect.x + 10, playerRect.y + 24, 12, 8};
                SDL_RenderFillRect(renderer, &leg);
            }
        }
        
        // UI - Score Box
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
        SDL_Rect scoreBox = {10, 10, 200, 35};
        SDL_RenderFillRect(renderer, &scoreBox);
        
        SDL_SetRenderDrawColor(renderer, 255, 220, 0, 255);
        SDL_RenderDrawRect(renderer, &scoreBox);
        
        // Score text
        if (gameFont) {
            char scoreText[32];
            snprintf(scoreText, sizeof(scoreText), "SCORE: %d", score);
            SDL_Color yellow = {255, 220, 0, 255};
            renderText(renderer, gameFont, scoreText, 20, 22, yellow, false);
        }
        
        // Lives Box (separate)
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
        SDL_Rect livesBox = {220, 10, 140, 35};
        SDL_RenderFillRect(renderer, &livesBox);
        
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &livesBox);
        
        // "LIVES:" label
        if (gameFont) {
            SDL_Color white = {255, 255, 255, 255};
            renderText(renderer, gameFont, "LIVES:", 230, 22, white, false);
        }
        
        // Draw hearts for lives
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        for (int i = 0; i < lives; i++) {
            SDL_Rect heart = {300 + i * 20, 17, 16, 16};
            SDL_RenderFillRect(renderer, &heart);
        }
        
        // Game Over Screen
        if (gameOver) {
            // Semi-transparent overlay
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
            SDL_Rect overlay = {0, 0, windowWidth, windowHeight};
            SDL_RenderFillRect(renderer, &overlay);
            
            // Game Over Box
            SDL_SetRenderDrawColor(renderer, 139, 0, 0, 255);
            SDL_Rect gameOverBox = {windowWidth / 2 - 250, windowHeight / 2 - 100, 500, 200};
            SDL_RenderFillRect(renderer, &gameOverBox);
            
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &gameOverBox);
            
            // Text
            if (gameFont) {
                SDL_Color white = {255, 255, 255, 255};
                renderText(renderer, gameFont, "GAME OVER", windowWidth / 2, windowHeight / 2 - 50, white, true);
                
                char finalScore[64];
                snprintf(finalScore, sizeof(finalScore), "FINAL SCORE: %d", score);
                renderText(renderer, gameFont, finalScore, windowWidth / 2, windowHeight / 2, white, true);
                
                renderText(renderer, gameFont, "Press ESC to return to menu", windowWidth / 2, windowHeight / 2 + 50, white, true);
            }
            
            // Auto return to menu after 5 seconds
            if (currentTime - deathTime > 5000) {
                if (gameFont) TTF_CloseFont(gameFont);
                return true;
            }
        }
        
        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }
    
    if (gameFont) TTF_CloseFont(gameFont);
    return true;
}
