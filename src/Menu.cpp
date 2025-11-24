    #include "Menu.h"
    #include <iostream>
    #include <cmath>
    #include <cstdlib>
    #include <ctime>

    Menu::Menu() 
        : selectedItem(0), titleFont(nullptr), itemFont(nullptr), smallFont(nullptr),
        pulsePhase(0.0f), fadeIn(0.0f), lastSelectTime(0), lastKeyTime(0),
        windowWidth(800), windowHeight(600) {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
    }

    Menu::~Menu() {
        cleanup();
    }

    bool Menu::init(int wWidth, int wHeight) {
        windowWidth = wWidth;
        windowHeight = wHeight;
        
        // Try to load fonts with multiple fallback paths
        const char* font_paths[] = {
            "assets/fonts/arial.ttf",
            "assets/PressStart2P-Regular.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            "/usr/share/fonts/TTF/DejaVuSans.ttf",
            "C:\\Windows\\Fonts\\arial.ttf"
        };
        
        const char* mono_paths[] = {
            "assets/fonts/DejaVuSansMono.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
            "/usr/share/fonts/TTF/DejaVuSansMono.ttf",
            "C:\\Windows\\Fonts\\consola.ttf"
        };
        
        // Load title font (large)
        for (const char* path : font_paths) {
            titleFont = TTF_OpenFont(path, 64);
            if (titleFont) break;
        }
        
        // Load item font (medium)
        for (const char* path : mono_paths) {
            itemFont = TTF_OpenFont(path, 32);
            if (itemFont) break;
        }
        
        // Load small font
        for (const char* path : mono_paths) {
            smallFont = TTF_OpenFont(path, 18);
            if (smallFont) break;
        }
        
        if (!titleFont || !itemFont || !smallFont) {
            std::cerr << "Warning: Some fonts failed to load. Install DejaVu fonts for best experience." << std::endl;
        }
        
        // Create menu items centered on screen
        int startY = windowHeight / 2 - 60;
        int spacing = 75;
        int itemWidth = 380;
        int itemHeight = 60;
        int itemX = (windowWidth - itemWidth) / 2;
        
        items.clear();
        items.push_back(MenuItem("SINGLEPLAYER", itemX, startY, itemWidth, itemHeight));
        items.push_back(MenuItem("HOST SERVER", itemX, startY + spacing, itemWidth, itemHeight));
        items.push_back(MenuItem("JOIN SERVER", itemX, startY + spacing * 2, itemWidth, itemHeight));
        items.push_back(MenuItem("SETTINGS", itemX, startY + spacing * 3, itemWidth, itemHeight));
        items.push_back(MenuItem("QUIT", itemX, startY + spacing * 4, itemWidth, itemHeight));
        
        initStarfield();
        lastSelectTime = SDL_GetTicks();
        
        std::cout << "Menu initialized with " << items.size() << " items" << std::endl;
        return true;
    }

    void Menu::initStarfield() {
        stars.clear();
        for (int i = 0; i < 100; ++i) {
            Star star;
            star.x = static_cast<float>(std::rand() % windowWidth);
            star.y = static_cast<float>(std::rand() % windowHeight);
            star.speed = 0.5f + static_cast<float>(std::rand() % 20) / 10.0f;
            star.brightness = 0.3f + static_cast<float>(std::rand() % 70) / 100.0f;
            stars.push_back(star);
        }
    }

    void Menu::handleEvent(SDL_Event& e, GameState& state, bool& running) {
        if (e.type == SDL_KEYDOWN) {
            handleKeyboard(e);
            if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_SPACE) {
                selectItem(state, running);
            }
        }
        
        handleMouse(e, state, running);
    }

    void Menu::handleKeyboard(SDL_Event& e) {
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastKeyTime < KEY_REPEAT_DELAY) return;
        
        switch (e.key.keysym.sym) {
            case SDLK_UP:
            case SDLK_w:
                lastKeyTime = currentTime;
                selectedItem = (selectedItem - 1 + items.size()) % items.size();
                lastSelectTime = currentTime;
                break;
                
            case SDLK_DOWN:
            case SDLK_s:
                lastKeyTime = currentTime;
                selectedItem = (selectedItem + 1) % items.size();
                lastSelectTime = currentTime;
                break;
                
            case SDLK_ESCAPE:
                lastKeyTime = currentTime;
                std::cout << "ESC pressed - exiting" << std::endl;
                break;
        }
    }

    void Menu::handleMouse(SDL_Event& e, GameState& state, bool& running) {
        if (e.type == SDL_MOUSEMOTION || e.type == SDL_MOUSEBUTTONDOWN) {
            int mx, my;
            SDL_GetMouseState(&mx, &my);
            
            for (size_t i = 0; i < items.size(); i++) {
                SDL_Rect& r = items[i].rect;
                bool wasHovered = items[i].hovered;
                
                if (mx >= r.x && mx <= r.x + r.w && my >= r.y && my <= r.y + r.h) {
                    items[i].hovered = true;
                    if (!wasHovered) {
                        selectedItem = i;
                        lastSelectTime = SDL_GetTicks();
                    }
                    
                    if (e.type == SDL_MOUSEBUTTONDOWN) {
                        selectItem(state, running);
                    }
                } else {
                    items[i].hovered = false;
                }
            }
        }
    }

    void Menu::selectItem(GameState& state, bool& running) {
        std::cout << "Selected: " << items[selectedItem].text << std::endl;
        
        switch (selectedItem) {
            case 0:  // START GAME
                state = PLAYING;
                std::cout << "[*] Starting single player game..." << std::endl;
                break;
                
            case 1:  // HOST SERVER
                state = HOSTING;
                std::cout << "[*] Hosting multiplayer server..." << std::endl;
                break;
                
            case 2:  // JOIN SERVER
                state = JOINING;
                std::cout << "[*] Joining multiplayer server..." << std::endl;
                break;
                
            case 3:  // SETTINGS
                state = SETTINGS;
                std::cout << "[*] Opening settings menu..." << std::endl;
                break;
                
            case 4:  // QUIT
                std::cout << "[*] Goodbye!" << std::endl;
                running = false;
                break;
        }
    }

    void Menu::update(float deltaTime) {
        // Fade in animation
        if (fadeIn < 1.0f) {
            fadeIn += deltaTime * 2.0f;
            if (fadeIn > 1.0f) fadeIn = 1.0f;
        }
        
        // Pulse animation
        pulsePhase += deltaTime * 2.0f;
        if (pulsePhase > 2.0f * M_PI) pulsePhase -= 2.0f * M_PI;
        
        // Update selection animation
        for (size_t i = 0; i < items.size(); ++i) {
            float target = (i == static_cast<size_t>(selectedItem)) ? 1.0f : 0.0f;
            float diff = target - items[i].selectAnim;
            items[i].selectAnim += diff * deltaTime * 8.0f;
        }
        
        // Animate starfield
        for (auto& star : stars) {
            star.y += star.speed * deltaTime * 60.0f;
            if (star.y > windowHeight) {
                star.y = 0;
                star.x = static_cast<float>(std::rand() % windowWidth);
            }
        }
    }

    void Menu::render(SDL_Renderer* renderer) {
        renderBackground(renderer);
        renderStarfield(renderer);
        renderTitle(renderer);
        renderItems(renderer);
        
        // Version info
        if (smallFont) {
            SDL_Color gray = {150, 150, 150, 200};
            renderText(renderer, "v1.0", 
                    windowWidth - 20, windowHeight - 30, smallFont, gray, false);
            renderText(renderer, "Press ESC to exit | WASD/Arrows to navigate", 
                    windowWidth / 2, windowHeight - 30, smallFont, gray, true);
        }
    }

    void Menu::renderBackground(SDL_Renderer* renderer) {
        // Gradient background from dark blue to darker purple
        for (int y = 0; y < windowHeight; ++y) {
            float t = static_cast<float>(y) / windowHeight;
            Uint8 r = static_cast<Uint8>(10 + t * 15);
            Uint8 g = static_cast<Uint8>(15 + t * 10);
            Uint8 b = static_cast<Uint8>(40 + t * 20);
            SDL_SetRenderDrawColor(renderer, r, g, b, 255);
            SDL_RenderDrawLine(renderer, 0, y, windowWidth, y);
        }
    }

    void Menu::renderStarfield(SDL_Renderer* renderer) {
        for (const auto& star : stars) {
            Uint8 alpha = static_cast<Uint8>(star.brightness * 255 * fadeIn);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, alpha);
            
            // Draw star with slight bloom
            SDL_RenderDrawPoint(renderer, static_cast<int>(star.x), static_cast<int>(star.y));
            if (star.brightness > 0.7f) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, alpha / 3);
                SDL_RenderDrawPoint(renderer, static_cast<int>(star.x + 1), static_cast<int>(star.y));
                SDL_RenderDrawPoint(renderer, static_cast<int>(star.x - 1), static_cast<int>(star.y));
                SDL_RenderDrawPoint(renderer, static_cast<int>(star.x), static_cast<int>(star.y + 1));
                SDL_RenderDrawPoint(renderer, static_cast<int>(star.x), static_cast<int>(star.y - 1));
            }
        }
    }

    void Menu::renderTitle(SDL_Renderer* renderer) {
        if (!titleFont) return;
        
        // Title with pulsing glow effect
        float pulse = 0.85f + 0.15f * std::sin(pulsePhase);
        SDL_Color cyan = {
            static_cast<Uint8>(100 * pulse),
            static_cast<Uint8>(200 * pulse),
            static_cast<Uint8>(255 * pulse),
            static_cast<Uint8>(255 * fadeIn)
        };
        
        // Shadow
        SDL_Color shadow = {0, 0, 0, static_cast<Uint8>(180 * fadeIn)};
        renderText(renderer, "GAMW", windowWidth / 2 + 3, 
                windowHeight / 6 + 3, titleFont, shadow, true);
        
        // Main title
        renderText(renderer, "GAMW", windowWidth / 2, 
                windowHeight / 6, titleFont, cyan, true);
        
        // Decorative line
        SDL_SetRenderDrawColor(renderer, 100, 150, 200, static_cast<Uint8>(150 * fadeIn));
        int lineY = windowHeight / 6 + 80;
        SDL_RenderDrawLine(renderer, windowWidth / 2 - 200, lineY, 
                        windowWidth / 2 + 200, lineY);
    }

    void Menu::renderItems(SDL_Renderer* renderer) {
        for (size_t i = 0; i < items.size(); i++) {
            bool isSelected = (i == static_cast<size_t>(selectedItem));
            renderMenuItem(renderer, items[i], isSelected);
        }
    }

    void Menu::renderMenuItem(SDL_Renderer* renderer, MenuItem& item, bool isSelected) {
        SDL_Rect& r = item.rect;
        float anim = easeInOutCubic(item.selectAnim);
        
        // Box background with transparency
        SDL_Color bgColor = {
            static_cast<Uint8>(20 + anim * 30),
            static_cast<Uint8>(30 + anim * 40),
            static_cast<Uint8>(60 + anim * 60),
            static_cast<Uint8>((180 + anim * 50) * fadeIn)
        };
        SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
        SDL_RenderFillRect(renderer, &r);
        
        // Outer border
        SDL_Color borderColor = {
            static_cast<Uint8>(60 + anim * 80),
            static_cast<Uint8>(100 + anim * 100),
            static_cast<Uint8>(160 + anim * 95),
            static_cast<Uint8>(255 * fadeIn)
        };
        SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, 
                            borderColor.b, borderColor.a);
        SDL_RenderDrawRect(renderer, &r);
        
        // Inner glow for selected item
        if (isSelected) {
            float glow = 0.5f + 0.5f * std::sin(pulsePhase * 2.0f);
            SDL_Rect inner = {r.x + 3, r.y + 3, r.w - 6, r.h - 6};
            SDL_SetRenderDrawColor(renderer, 100, 180, 255, 
                                static_cast<Uint8>(glow * 100 * fadeIn));
            SDL_RenderDrawRect(renderer, &inner);
            
            // Side indicators
            int centerY = r.y + r.h / 2;
            int offset = static_cast<int>(6.0f * std::sin(pulsePhase * 2.0f));
            
            SDL_SetRenderDrawColor(renderer, 100, 200, 255, static_cast<Uint8>(255 * fadeIn));
            
            // Left arrow
            SDL_RenderDrawLine(renderer, r.x - 25 - offset, centerY - 8, 
                            r.x - 15 - offset, centerY);
            SDL_RenderDrawLine(renderer, r.x - 15 - offset, centerY, 
                            r.x - 25 - offset, centerY + 8);
            
            // Right arrow
            SDL_RenderDrawLine(renderer, r.x + r.w + 25 + offset, centerY - 8, 
                            r.x + r.w + 15 + offset, centerY);
            SDL_RenderDrawLine(renderer, r.x + r.w + 15 + offset, centerY, 
                            r.x + r.w + 25 + offset, centerY + 8);
        }
        
        // Render text
        if (itemFont) {
            SDL_Color textColor = {
                static_cast<Uint8>(200 + anim * 55),
                static_cast<Uint8>(220 + anim * 35),
                static_cast<Uint8>(240 + anim * 15),
                static_cast<Uint8>(255 * fadeIn)
            };
            
            renderText(renderer, item.text.c_str(), r.x + r.w / 2, 
                    r.y + r.h / 2 - 12, itemFont, textColor, true);
        }
    }

    void Menu::renderText(SDL_Renderer* renderer, const char* text, int x, int y, 
                        TTF_Font* font, SDL_Color color, bool centered) {
        if (!font) return;
        
        SDL_Surface* surface = TTF_RenderText_Blended(font, text, color);
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_Rect rect = {
                centered ? x - surface->w / 2 : x,
                y,
                surface->w,
                surface->h
            };
            SDL_RenderCopy(renderer, texture, nullptr, &rect);
            SDL_DestroyTexture(texture);
            SDL_FreeSurface(surface);
        }
    }

    float Menu::easeInOutCubic(float t) {
        return t < 0.5f ? 4.0f * t * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 3.0f) / 2.0f;
    }

    SDL_Color Menu::lerpColor(SDL_Color a, SDL_Color b, float t) {
        return {
            static_cast<Uint8>(a.r + (b.r - a.r) * t),
            static_cast<Uint8>(a.g + (b.g - a.g) * t),
            static_cast<Uint8>(a.b + (b.b - a.b) * t),
            static_cast<Uint8>(a.a + (b.a - a.a) * t)
        };
    }

    void Menu::cleanup() {
        if (titleFont) {
            TTF_CloseFont(titleFont);
            titleFont = nullptr;
        }
        if (itemFont) {
            TTF_CloseFont(itemFont);
            itemFont = nullptr;
        }
        if (smallFont) {
            TTF_CloseFont(smallFont);
            smallFont = nullptr;
        }
    }