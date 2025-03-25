#include "VEGA1.h"
#include <iostream>
#include <vector>

const int FPS_CAP = 60; 
const int FRAME_DELAY = 1000 / FPS_CAP;
VEGA16::VEGA16() : running(false), window(nullptr), renderer(nullptr), frameCount(0), fps(0), lastTime(0), isCrushed(false) {
    player = {SCREEN_WIDTH / 2 - BLOCK_SIZE / 2, SCREEN_HEIGHT - BLOCK_SIZE - 90, BLOCK_SIZE, BLOCK_SIZE};
}

bool VEGA16::init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    window = SDL_CreateWindow("VEGA¹⁶ Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return false;
    }
    font = TTF_OpenFont("font.ttf", 24);
    if (!font) {
        std::cerr << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return false;
    }
    running = true;
    lastTime = SDL_GetTicks();
    return true;
}

bool isJumping = false;  
int jumpHeight = 0; 
int verticalVelocity = 0;  
const int gravity = 1; 
const int jumpSpeed = 15; 
int jumpCount = 0;  
const int doubleJumpHeight = 120;  
const int maxJumpHeight = 100;  

void VEGA16::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
        }

        if (!isCrushed) {
            if (event.type == SDL_KEYDOWN) {
                const int moveSpeed = 5;
                const int screen_width_limit = SCREEN_WIDTH - BLOCK_SIZE;

                if (event.key.keysym.sym == SDLK_w) {
                    if (!isJumping) {
                        verticalVelocity = -jumpSpeed;  
                        isJumping = true;
                        jumpCount = 1;  
                    } else if (jumpCount == 1) {  
                        verticalVelocity = -jumpSpeed * 2;  
                        jumpCount = 2;  
                    }
                }

                if (event.key.keysym.sym == SDLK_a && player.x > 0) {
                    player.x -= moveSpeed;
                }
                if (event.key.keysym.sym == SDLK_d && player.x < screen_width_limit) {
                    player.x += moveSpeed;
                }

                if (event.key.keysym.sym == SDLK_s) {
                    if (player.y + BLOCK_SIZE >= SCREEN_HEIGHT - 90) {
                        isCrushed = true;
                    } else {
                        player.y += moveSpeed;
                    }
                }
            }
        }

        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN && isCrushed) {
            restartGame();
        }
    }
}

void VEGA16::update() {
    if (isJumping || jumpCount == 2) {
        verticalVelocity += gravity;  
        player.y += verticalVelocity;  

        if (verticalVelocity >= 0 && player.y >= SCREEN_HEIGHT - BLOCK_SIZE - 90) {
            player.y = SCREEN_HEIGHT - BLOCK_SIZE - 90;  
            isJumping = false;  
            jumpCount = 0;  
            verticalVelocity = 0;  
        }
    } else {
        if (player.y < SCREEN_HEIGHT - BLOCK_SIZE - 90) {
            player.y += verticalVelocity;
        }
    }
}

void VEGA16::render() {
    Uint32 frameStart = SDL_GetTicks();  

    SDL_SetRenderDrawColor(renderer, 135, 206, 235, 255);  
    SDL_RenderClear(renderer);  

    SDL_SetRenderDrawColor(renderer, 65, 153, 10, 255);  
    SDL_Rect groundRect = {0, SCREEN_HEIGHT - 50, SCREEN_WIDTH, 50};  
    SDL_RenderFillRect(renderer, &groundRect);  

    drawTree(100, SCREEN_HEIGHT - 50);  
    drawTree(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT - 50);  
    drawTree(SCREEN_WIDTH - 200, SCREEN_HEIGHT - 50);  

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  

    int headRadius = 15;
    int centerX = player.x + player.w / 2;
    int centerY = player.y + headRadius;

    for (int angle = 0; angle < 360; angle++) {
        int x = centerX + headRadius * cos(angle * M_PI / 180);
        int y = centerY + headRadius * sin(angle * M_PI / 180);
        SDL_RenderDrawPoint(renderer, x, y);  
    }

    int torsoLength = 40;  
    SDL_RenderDrawLine(renderer, centerX, centerY + headRadius, centerX, centerY + headRadius + torsoLength);  

    SDL_RenderDrawLine(renderer, centerX, centerY + headRadius + torsoLength, centerX - 15, centerY + headRadius + torsoLength + 20);  
    SDL_RenderDrawLine(renderer, centerX, centerY + headRadius + torsoLength, centerX + 15, centerY + headRadius + torsoLength + 20);  

    if (isCrushed) {
        displayMessage("YOU WERE CRUSHED!", SCREEN_HEIGHT / 2 - 40);  
        displayMessage("Press ENTER to Restart", SCREEN_HEIGHT / 2 + 40);  
    }

    checkTreeCollisions();

    SDL_RenderPresent(renderer);  

    frameCount++;
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - lastTime >= 1000) {
        fps = frameCount;  
        frameCount = 0;    
        lastTime = currentTime;  

        std::cout << "FPS: " << fps << std::endl;
    }

    Uint32 frameTime = SDL_GetTicks() - frameStart;  
    if (frameTime < FRAME_DELAY) {
        SDL_Delay(FRAME_DELAY - frameTime);  
    }
}

void VEGA16::drawTree(int x, int y) {
    SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255);  

    SDL_Rect trunkRect = {x - 50, y - 250, 100, 250};  
    SDL_RenderFillRect(renderer, &trunkRect);

    SDL_SetRenderDrawColor(renderer, 0, 128, 0, 255);  

    SDL_Rect leavesBottom = {x - 120, y - 300, 240, 60};  
    SDL_RenderFillRect(renderer, &leavesBottom);

    SDL_Rect leavesMiddle = {x - 90, y - 350, 180, 60};  
    SDL_RenderFillRect(renderer, &leavesMiddle);

    SDL_Rect leavesTop = {x - 60, y - 400, 120, 60};  
    SDL_RenderFillRect(renderer, &leavesTop);
}

void VEGA16::checkTreeCollisions() {
    std::vector<int> treeXPositions = {100, SCREEN_WIDTH / 2 - 100, SCREEN_WIDTH - 200};
    std::vector<SDL_Rect> treeTrunks;
    std::vector<SDL_Rect> treeLeaves;

    for (int x : treeXPositions) {
        int trunkY = SCREEN_HEIGHT - 50 - 250;
        treeTrunks.push_back({x - 50, trunkY, 100, 250});  

        treeLeaves.push_back({x - 120, trunkY - 50, 240, 60});  
        treeLeaves.push_back({x - 90, trunkY - 100, 180, 60});  
        treeLeaves.push_back({x - 60, trunkY - 150, 120, 60});  
    }

    for (const auto& trunk : treeTrunks) {
        if (SDL_HasIntersection(&player, &trunk)) {
            isCrushed = true;  
            return;
        }
    }

    bool isOnLeaves = false;  

    for (const auto& leaf : treeLeaves) {
        if (SDL_HasIntersection(&player, &leaf)) {
            if (player.y + player.h <= leaf.y + leaf.h && verticalVelocity > 0) {
                player.y = leaf.y - player.h;
                isJumping = false;  
                verticalVelocity = 0;  
                isOnLeaves = true;  
            } 
            else if (verticalVelocity < 0) {
                continue;  
            }
        }
    }

    if (!isOnLeaves && !isJumping) {
        player.y += verticalVelocity;
    }

    prevPlayerX = player.x;
    prevPlayerY = player.y;
}

void VEGA16::displayMessage(const char* message, int yOffset) {
    SDL_Color white = {255, 255, 255};
    SDL_Surface* surface = TTF_RenderText_Solid(font, message, white);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    int textWidth = surface->w;
    int textHeight = surface->h;
    SDL_Rect textRect = {(SCREEN_WIDTH - textWidth) / 2, yOffset, textWidth, textHeight};
    SDL_RenderCopy(renderer, texture, nullptr, &textRect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void VEGA16::restartGame() {
    clean();

    if (!init()) {
        std::cerr << "Failed to restart the game!" << std::endl;
        running = false;
    } else {
        player.x = SCREEN_WIDTH / 2 - BLOCK_SIZE / 2;
        player.y = SCREEN_HEIGHT - BLOCK_SIZE - 90;  
        isCrushed = false;  
    }
}

void VEGA16::clean() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);  
    SDL_Quit();
    TTF_Quit();  
}

