#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include "Menu.h"
#include "GameBox.h"

class Game {
public:
    Game() : window(nullptr), renderer(nullptr), running(true), 
             state(MENU), fullscreen(false), lastFrameTime(0) {}
    
    ~Game() {
        cleanup();
    }
    
    bool init() {
        // Wayland compatibility
        SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
        SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
        
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
            return false;
        }
        
        if (TTF_Init() == -1) {
            std::cerr << "TTF_Init Error: " << TTF_GetError() << std::endl;
            return false;
        }
        
        // Get desktop display mode
        SDL_DisplayMode dm;
        if (SDL_GetDesktopDisplayMode(0, &dm) != 0) {
            std::cerr << "SDL_GetDesktopDisplayMode Error: " << SDL_GetError() << std::endl;
            windowWidth = 1280;
            windowHeight = 720;
        } else {
            windowWidth = dm.w;
            windowHeight = dm.h;
        }
        
        // Create window
        Uint32 windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI;
        if (fullscreen) {
            windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        }
        
        window = SDL_CreateWindow(
            "Gamw",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            fullscreen ? windowWidth : 1280,
            fullscreen ? windowHeight : 720,
            windowFlags
        );
        
        if (!window) {
            std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
            return false;
        }
        
        // Create renderer
        renderer = SDL_CreateRenderer(window, -1, 
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        
        if (!renderer) {
            std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
            return false;
        }
        
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        
        // Get actual window size
        SDL_GetWindowSize(window, &windowWidth, &windowHeight);
        
        // Initialize menu
        if (!menu.init(windowWidth, windowHeight)) {
            std::cerr << "Menu initialization failed" << std::endl;
            return false;
        }
        
        lastFrameTime = SDL_GetTicks();
        
        std::cout << "========================================" << std::endl;
        std::cout << "Gamw" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "Window: " << windowWidth << "x" << windowHeight << std::endl;
        std::cout << "Video Driver: " << SDL_GetCurrentVideoDriver() << std::endl;
        std::cout << "Renderer: " << SDL_GetRendererInfo(renderer, &rendererInfo) << std::endl;
        std::cout << "========================================" << std::endl;
        
        return true;
    }
    
    void run() {
        const int TARGET_FPS = 60;
        const int FRAME_DELAY = 1000 / TARGET_FPS;
        
        Uint32 frameStart;
        int frameTime;
        
        while (running) {
            frameStart = SDL_GetTicks();
            
            handleEvents();
            update();
            render();
            
            frameTime = SDL_GetTicks() - frameStart;
            
            if (FRAME_DELAY > frameTime) {
                SDL_Delay(FRAME_DELAY - frameTime);
            }
        }
    }
    
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_RendererInfo rendererInfo;
    Menu menu;
    bool running;
    GameState state;
    bool fullscreen;
    int windowWidth;
    int windowHeight;
    Uint32 lastFrameTime;
    
    void handleEvents() {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
            }
            else if (e.type == SDL_KEYDOWN) {
                // Global hotkeys
                if (e.key.keysym.sym == SDLK_F11) {
                    toggleFullscreen();
                }
                else if (e.key.keysym.sym == SDLK_ESCAPE && state == MENU) {
                    running = false;
                }
                else if (e.key.keysym.sym == SDLK_ESCAPE && state != MENU) {
                    // Return to menu from other states
                    state = MENU;
                    std::cout << "[*] Returning to menu" << std::endl;
                }
            }
            
            // Pass events to menu
            if (state == MENU) {
                menu.handleEvent(e, state, running);
            }
        }
    }
    
    void update() {
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastFrameTime) / 1000.0f;
        lastFrameTime = currentTime;
        
        if (state == MENU) {
            menu.update(deltaTime);
        }
        else if (state == PLAYING) {
            if (!runGameBox(renderer)) {
            state = MENU; 
        std::cout << "[*] Returning from game to menu" << std::endl;
    }
        }
        else if (state == HOSTING) {
            // Server hosting logic here
        }
        else if (state == JOINING) {
            // Client joining logic here
        }
        else if (state == SETTINGS) {
            // Settings logic here
        }
    }
    
    void render() {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        
        if (state == MENU) {
            menu.render(renderer);
        }
        else if (state == PLAYING) {
            renderPlaying();
        }
        else if (state == HOSTING) {
            renderHosting();
        }
        else if (state == JOINING) {
            renderJoining();
        }
        else if (state == SETTINGS) {
            renderSettings();
        }
        
        SDL_RenderPresent(renderer);
    }
    
    void renderPlaying() {
        // Placeholder for game rendering
        SDL_SetRenderDrawColor(renderer, 20, 40, 60, 255);
        SDL_RenderClear(renderer);
        
        // Draw simple text
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect rect = {windowWidth/2 - 150, windowHeight/2 - 50, 300, 100};
        SDL_RenderFillRect(renderer, &rect);
        
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &rect);
    }
    
    void renderHosting() {
        // Placeholder for hosting screen
        SDL_SetRenderDrawColor(renderer, 40, 20, 60, 255);
        SDL_RenderClear(renderer);
        
        SDL_SetRenderDrawColor(renderer, 200, 150, 255, 255);
        SDL_Rect rect = {windowWidth/2 - 150, windowHeight/2 - 50, 300, 100};
        SDL_RenderFillRect(renderer, &rect);
    }
    
    void renderJoining() {
        // Placeholder for joining screen
        SDL_SetRenderDrawColor(renderer, 20, 60, 40, 255);
        SDL_RenderClear(renderer);
        
        SDL_SetRenderDrawColor(renderer, 150, 255, 200, 255);
        SDL_Rect rect = {windowWidth/2 - 150, windowHeight/2 - 50, 300, 100};
        SDL_RenderFillRect(renderer, &rect);
    }
    
    void renderSettings() {
        // Placeholder for settings screen
        SDL_SetRenderDrawColor(renderer, 60, 40, 20, 255);
        SDL_RenderClear(renderer);
        
        SDL_SetRenderDrawColor(renderer, 255, 200, 150, 255);
        SDL_Rect rect = {windowWidth/2 - 150, windowHeight/2 - 50, 300, 100};
        SDL_RenderFillRect(renderer, &rect);
    }
    
    void toggleFullscreen() {
        fullscreen = !fullscreen;
        
        if (fullscreen) {
            SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
            SDL_GetWindowSize(window, &windowWidth, &windowHeight);
            std::cout << "[*] Fullscreen enabled: " << windowWidth << "x" << windowHeight << std::endl;
        } else {
            SDL_SetWindowFullscreen(window, 0);
            windowWidth = 1280;
            windowHeight = 720;
            SDL_SetWindowSize(window, windowWidth, windowHeight);
            SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
            std::cout << "[*] Windowed mode: " << windowWidth << "x" << windowHeight << std::endl;
        }
        
        // Reinitialize menu with new dimensions
        menu.init(windowWidth, windowHeight);
    }
    
    void cleanup() {
        menu.cleanup();
        
        if (renderer) {
            SDL_DestroyRenderer(renderer);
            renderer = nullptr;
        }
        
        if (window) {
            SDL_DestroyWindow(window);
            window = nullptr;
        }
        
        TTF_Quit();
        SDL_Quit();
        
        std::cout << "[*] Cleanup complete" << std::endl;
    }
};

int main(int argc, char* argv[]) {
    std::cout << "Starting Gamw..." << std::endl;
    
    Game game;
    
    if (!game.init()) {
        std::cerr << "[!] Failed to initialize game" << std::endl;
        return 1;
    }
    
    game.run();
    
    std::cout << "Game closed successfully" << std::endl;
    return 0;
}