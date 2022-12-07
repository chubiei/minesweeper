#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "game.h"

class CmdUI {
    private:
        MineGame *game;
        int** game_grid;
        int height;

    public:
        CmdUI(MineGame *game);
        ~CmdUI();

        void ProcessCommands();

    private:
        void GetTokens(std::string tokens[4]);
        void Resize();
        void ClearGrid();
        void UpdateGrid();
        char GetGridState(int x, int y);
        void Redraw();
        void ShowHelp();
};

CmdUI::CmdUI(MineGame *game)
{
    this->game = game;
    this->game_grid = NULL;
    this->height = 0;
}

CmdUI::~CmdUI()
{
    if (this->game_grid != NULL) {
        for (int i = 0; i < this->height; i++) {
            delete [] this->game_grid[i];
        }

        delete [] this->game_grid;
        this->game_grid = NULL;
        this->height = 0;
    }
}

void CmdUI::GetTokens(std::string tokens[4])
{
    char buffer[8192];
    std::stringstream ss;

    // read entire line into buffer and we use stringstream to do tokenization
    std::cin.getline(buffer, sizeof(buffer));
    ss << buffer;

    for (int i = 0; i < 4; i++) {
        if (ss.good()) {
            ss >> tokens[i];
        }
    }
}

void CmdUI::ProcessCommands()
{
    this->Resize();
    this->ClearGrid();
    this->Redraw();

    while (1) {
        std::string tokens[4];
        std::string command;

        std::cout << "command> ";

        this->GetTokens(tokens);
        command = tokens[0];

        // reset
        if (command == "reset") {
            this->game->Reset();
            this->ClearGrid();
            this->Redraw();
        }
        // set-game
        else if (command == "set-game") {
            int width = std::atoi(tokens[1].c_str());
            int height = std::atoi(tokens[2].c_str());
            int mine = std::atoi(tokens[3].c_str());

            this->game->SetCustom(width, height, mine);
            this->Resize();
            this->ClearGrid();
            this->Redraw();
        }
        // flag
        else if (command == "flag" || command == "f") {
            int x = std::atoi(tokens[1].c_str());
            int y = std::atoi(tokens[2].c_str());

            this->game->TouchFlag(x, y);
            this->UpdateGrid();
            this->Redraw();
        } 
        // open
        else if (command == "open" || command == "o") {
            int x = std::atoi(tokens[1].c_str());
            int y = std::atoi(tokens[2].c_str());

            this->game->Open(x, y);
            this->UpdateGrid();
            this->Redraw();
        } else if (command == "quit" || command == "q" || command == "exit") {
            break;
        } else if (command == "show" || command == "s" || command == "info" || command == "i") {
            this->Redraw();
        } else {
            this->ShowHelp();
        }
    }
}

void CmdUI::Resize()
{
    // delete old
    if (this->game_grid != NULL) {
        for (int i = 0; i < this->height; i++) {
            delete [] this->game_grid[i];
        }

        delete [] this->game_grid;
        this->game_grid = NULL;
        this->height = 0;
    }

    // new
    this->game_grid = new int* [this->game->GetHeight()];
    for (int i = 0; i < this->game->GetHeight(); i++) {
        this->game_grid[i] = new int [this->game->GetWidth()];
    }

    this->height = this->game->GetHeight();
}

void CmdUI::ClearGrid()
{
    for (int y = 0; y < this->game->GetHeight(); y++) {
        for (int x = 0; x < this->game->GetWidth(); x++) {
            this->game_grid[y][x] = MineGameGrid::State::STATE_COVERED;
        }
    }
}

void CmdUI::UpdateGrid()
{
    std::vector<MineGameGrid> grids;

    this->game->GetDirtyGrids(grids);
    this->game->ClearDirtyGrids();

    for (int i = 0; i < grids.size(); i++) {
        int x = grids[i].x;
        int y = grids[i].y;

        this->game_grid[y][x] = grids[i].state;
    }
}

char CmdUI::GetGridState(int x, int y)
{
    // NOTE: MineGameGrid::State is carefully designed so that we can convert it quick
    char map[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '_', 'F', 'F', 'X', 'M'};
    int index = this->game_grid[y][x];

    return map[index];
}

void CmdUI::Redraw()
{
    // =================================
    int width = this->game->GetWidth() * 2;
    for (int i = 0; i < width; i++) {
        std::cout << "=";
    }

    std::cout << std::endl;

    // grid
    for (int y = 0; y < this->game->GetHeight(); y++) {
        for (int x = 0; x < this->game->GetWidth(); x++) {
            char c = this->GetGridState(x, y);

            std::cout << c << " ";
        }

        std::cout << std::endl;
    }

    // =================================
    for (int i = 0; i < width; i++) {
        std::cout << "=";
    }

    std::cout << std::endl;

    // flag count and mine count
    int flag_count = this->game->GetFlagCount();
    std::cout << "Flags: " << flag_count << ", Remaining mines: " << (this->game->GetMineCount() - flag_count) << std::endl;

    int game_state = this->game->GetGameState();
    if (game_state == MineGame::State::GAME_WON) {
        std::cout << "Congratulations! You win!" << std::endl;
    } else if (game_state == MineGame::State::GAME_LOST) {
        std::cout << "You loose, try it again." << std::endl;
    }
}

void CmdUI::ShowHelp()
{
    std::cout << "Command line Minesweeper available commands" << std::endl;
    std::cout << std::endl;
    std::cout << "quit|exit|q              Quit this game." << std::endl;
    std::cout << "reset                    Reset game to initial state." << std::endl;
    std::cout << "show|s                   Show game information." << std::endl;
    std::cout << "flag|f <x> <y>           Place or remove a flag at (x, y). The upper left is defined to (0, 0)" << std::endl;
    std::cout << "open|o <x> <y>           Open a grid at (x, y). The upper left is defined to (0, 0)" << std::endl;
    std::cout << "set-game <w> <h> <m>     Set difficulty to width=w, height=h, mine count=m." << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << std::endl;
    std::cout << "command> open 0 1" << std::endl;
    std::cout << "Opens grid at (0, 1)" << std::endl;
    std::cout << std::endl;
    std::cout << "command> f 10 20" << std::endl;
    std::cout << "Place a flag at (10, 20)" << std::endl;
    std::cout << "Give the same command will remove the flag at (10, 20)" << std::endl;
    std::cout << std::endl;
    std::cout << "command> set-game 9 9 10" << std::endl;
    std::cout << "Set game to have 10 mines with width=10 and height=10. " << std::endl;
    std::cout << "The difficulty will be the same as the beginner level." << std::endl;
}

//////////////////////////////////////////////////////////////////

int main()
{
    CmdUI *ui;
    MineGame *game;

    game = new MineGame();
    game->SetBeginner();

    // create a command line UI, interact with game through UI
    ui = new CmdUI(game);
    ui->ProcessCommands();

    delete ui;
    delete game;

    return 0;
}