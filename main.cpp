#include <iostream>
#include "sdl_headers.h"
#include "ui.h"
#include "game.h"

int main(int argc, char** argv)
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        std::cerr << "SDL_Init failed with error: " << SDL_GetError() << std::endl;
        return -1;
    }

    MineGameWindowUI *ui;
    MineGame *game;

    // create game
    game = new MineGame();
    game->SetExpert();

    // create UI, interact with game through UI
    ui = new MineGameWindowUI(game);
    ui->CreateComponents();
    ui->ProcessEvents();
    ui->DestroyComponents();

    delete ui;
    delete game;

    // Quit SDL subsystems
    SDL_Quit();

    return 0;
}
