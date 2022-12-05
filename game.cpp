#include <iostream>
#include <iomanip>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include "game.h"

struct Point {
    int x;
    int y;
};

MineGame::MineGame()
{
    this->width = 0;
    this->height = 0;
    this->mine_map = NULL;
    this->grid_state_map = NULL;
    this->grid_dirty_map = NULL;
    this->mine_count = 0;
    this->flag_count = 0;
    this->remaining_count = 0;
    this->game_state = MineGame::State::GAME_READY;

    this->SetBeginner();
}

MineGame::~MineGame()
{
    this->FreeMap();
}

void MineGame::SetBeginner()
{
    this->SetCustom(9, 9, 10);
}

void MineGame::SetIntermediate()
{
    this->SetCustom(16, 16, 40);
}

void MineGame::SetExpert()
{
    this->SetCustom(30, 16, 99);
}

void MineGame::SetCustom(int width, int height, int mine_count)
{
    std::cout << "SetCustom(width=" << width << ", height=" << height << ", mine_count= " << mine_count << ")" << std::endl;

    // make sure the inputs are correct
    width = (width > 50) ? 50 : width;
    height = (height > 50) ? 50 : height;
    mine_count = (mine_count >= width * height) ? (width * height - 1) : mine_count;

    this->FreeMap();
    this->AllocateMap(width, height);

    this->width = width;
    this->height = height;
    this->mine_count = mine_count;

    this->flag_count = 0;
    this->remaining_count = this->width * this->height - this->mine_count;
    this->game_state = MineGame::State::GAME_READY;

    // clean up map
    for (int i = 0; i < this->height; i++) {
        for (int j = 0; j < this->width; j++) {
            this->mine_map[i][j] = 0;
            this->grid_state_map[i][j] = MineGameGrid::State::STATE_COVERED;
            this->grid_dirty_map[i][j] = 0;
        }
    }
}

void MineGame::Reset()
{
    this->flag_count = 0;
    this->remaining_count = this->width * this->height - this->mine_count;
    this->game_state = MineGame::State::GAME_READY;

    // clean up map
    for (int i = 0; i < this->height; i++) {
        for (int j = 0; j < this->width; j++) {
            this->mine_map[i][j] = 0;
            this->grid_state_map[i][j] = MineGameGrid::State::STATE_COVERED;
            this->grid_dirty_map[i][j] = 0;
        }
    }
}

MineGame::State MineGame::GetGameState()
{
    return this->game_state;
}

MineGameGrid::State MineGame::GetGridState(int x, int y)
{
    return this->grid_state_map[y][x];
}

void MineGame::GetDirtyGrids(std::vector<MineGameGrid> &grids)
{
    for (int y = 0; y < this->height; y++) {
        for (int x = 0; x < this->width; x++) {
            if (this->IsDirtyGrid(x, y)) {
                MineGameGrid g;

                g.state = this->GetGridState(x, y);
                g.x = x;
                g.y = y;
                grids.push_back(g);
            }
        }
    }
}

void MineGame::ClearDirtyGrids()
{
    for (int y = 0; y < this->height; y++) {
        for (int x = 0; x < this->width; x++) {
            grid_dirty_map[y][x] = 0;
        }
    }
}

void MineGame::TouchFlag(int x, int y)
{
    MineGameGrid e;

    std::cout << "MineGame::TouchFlag(x=" << x << ", y=" << y << ")" << std::endl;

    if (!this->IsValidPoint(x, y)) {
        std::cerr << "MineGame::TouchFlag(x=" << x << ", y=" << y << "): run time error due to invalid input" << std::endl;
        return;
    }

    if (this->game_state == MineGame::State::GAME_RUNNING || this->game_state == MineGame::State::GAME_READY) {
        if (this->GetGridState(x, y) == MineGameGrid::State::STATE_COVERED) {
            this->SetGridState(x, y, MineGameGrid::State::STATE_FLAGGED);
            this->flag_count += 1;
        } else if (this->GetGridState(x, y) == MineGameGrid::State::STATE_FLAGGED) {
            this->SetGridState(x, y, MineGameGrid::State::STATE_COVERED);
            this->flag_count -= 1;
        }
    }
}

void MineGame::Open(int x, int y)
{
    std::cout << "MineGame::Open(x=" << x << ", y=" << y << ")" << std::endl;

    if (!this->IsValidPoint(x, y)) {
        std::cerr << "MineGame::Open(x=" << x << ", y=" << y << "): run time error due to invalid input" << std::endl;
        return;
    }

    // lazy init
    if (this->game_state == MineGame::State::GAME_READY) {
        this->InitMines(x, y);

        // this is for debugging
        for (int y = 0; y < this->height; y++) {
            for (int x = 0; x < this->width; x++) {
                std::cout << std::setw(4) << this->mine_map[y][x];
            }

            std::cout << std::endl;
        }
    }

    if (this->game_state == MineGame::State::GAME_RUNNING) {
        if (this->GetGridState(x, y) == MineGameGrid::State::STATE_COVERED) {
            std::cout << "debug: x=" << x << ", y=" << y << ", has_mine = " << this->HasMine(x, y) << ", mine_map = " << this->mine_map[y][x] << std::endl;

            if (this->HasMine(x, y)) {
                this->EndGame(x, y);
            } else {
                this->OpenRecursive(x, y);

                if (this->remaining_count == 0) {
                    this->WinGame();
                }
            }
        }
    }
}

void MineGame::OpenFast(int x, int y)
{
}

////////////////////////////////////////////////////////////////////////////////////

void MineGame::InitMines(int skip_x, int skip_y)
{
    std::cout << "InitMines(skip_x=" << skip_x << ", skip_y=" << skip_y << ")" << std::endl;

    std::srand (unsigned(std::time(0)));

    // place mines randomly with random_shuffle
    int map_size = this->width * this->height - 1;
    int map[map_size];

    for (int i = 0; i < this->mine_count; i++) {
        // -1: mine
        map[i] = -1;
    }

    for (int i = this->mine_count; i < map_size; i++) {
        // -2: unknown
        map[i] = -2;
    }

    std::random_shuffle(map, map + map_size);

    // place mines to the correct location on map
    int skip = skip_y * this->width + skip_x;

    for (int y = 0; y < this->height; y++) {
        for (int x = 0; x < this->width; x++) {
            int z = y * this->width + x;

            if (z < skip) {
                this->mine_map[y][x] = map[z];
            } else if (z > skip) {
                this->mine_map[y][x] = map[z - 1];
            } else {
                this->mine_map[y][x] = -2;
            }
        }
    }

    // calculate neighbors and complete both maps
    for (int y = 0; y < this->height; y++) {
        for (int x = 0; x < this->width; x++) {
            if (this->mine_map[y][x] != -1) {
                int up = y - 1, down = y + 1, left = x - 1, right = x + 1;
                int count = 0;

                if (this->HasMine(left, up)) count++;
                if (this->HasMine(left, y)) count++;
                if (this->HasMine(left, down)) count++;
                if (this->HasMine(x, up)) count++;
                if (this->HasMine(x, down)) count++;
                if (this->HasMine(right, up)) count++;
                if (this->HasMine(right, y)) count++;
                if (this->HasMine(right, down)) count++;

                this->mine_map[y][x] = count;
            }

            // NOTE: InitMines does not change grid_state_map, because flags can be placed prior to init 
            // this->grid_state_map[y][x] = MineGameGrid::State::STATE_COVERED;
        }
    }

    // NOTE: InitMines does not change flag_count, because flags can be placed prior to init 
    //this->flag_count = 0;
    this->remaining_count = this->width * this->height - this->mine_count;
    this->game_state = MineGame::State::GAME_RUNNING;
}

int MineGame::AllocateMap(int width, int height)
{
    if (width > 0 && height > 0) {
        // allocate mine map and flag map
        this->mine_map = new int* [height];
        this->grid_state_map = new MineGameGrid::State* [height];
        this->grid_dirty_map = new int* [height];

        for (int i = 0; i < height; i++) {
            this->mine_map[i] = new int [width];
            this->grid_state_map[i] = new MineGameGrid::State [width];
            this->grid_dirty_map[i] = new int [width];
        }

        this->width = width;
        this->height = height;
    }

    return 0;
}

void MineGame::FreeMap()
{
    // free mine map
    if (this->mine_map != NULL) {
        for (int i = 0; i < this->height; i++) {
            delete [] this->mine_map[i];
        }

        delete [] this->mine_map;
        this->mine_map = NULL;
    }

    // free flag map
    if (this->grid_state_map != NULL) {
        for (int i = 0; i < this->height; i++) {
            delete [] this->grid_state_map[i];
        }

        delete [] this->grid_state_map;
        this->grid_state_map = NULL;
    }

    // free dirty map
    if (this->grid_dirty_map != NULL) {
        for (int i = 0; i < this->height; i++) {
            delete [] this->grid_dirty_map[i];
        }

        delete [] this->grid_dirty_map;
        this->grid_dirty_map = NULL;
    }

    this->width = 0;
    this->height = 0;
}

void MineGame::EndGame(int explode_x, int explode_y)
{
    this->SetGridState(explode_x, explode_y, MineGameGrid::State::STATE_MINE_EXPLODE);

    // uncovered all mines
    for (int y = 0; y < this->height; y++) {
        for (int x = 0; x < this->width; x++) {
            if (this->HasMine(x, y) && this->GetGridState(x, y) == MineGameGrid::State::STATE_COVERED) {
                this->SetGridState(x, y, MineGameGrid::State::STATE_MINE_OPEN);
            }
        }
    }

    this->game_state = MineGame::State::GAME_LOST;
    std::cout << "You've lost" << std::endl;
}

void MineGame::WinGame()
{
    // show all uncovered flags
    for (int y = 0; y < this->height; y++) {
        for (int x = 0; x < this->width; x++) {
            if (this->HasMine(x, y) && this->GetGridState(x, y) == MineGameGrid::State::STATE_COVERED) {
                this->SetGridState(x, y, MineGameGrid::State::STATE_FLAGGED);
            }
        }
    }

    this->flag_count = this->mine_count;

    this->game_state = MineGame::State::GAME_WON;
    std::cout << "You won!!!" << std::endl;
}

void MineGame::OpenRecursive(int x, int y)
{
    if (!this->IsValidPoint(x, y)) {
        return;
    }

    if (this->grid_state_map[y][x] != MineGameGrid::State::STATE_COVERED && this->grid_state_map[y][x] != MineGameGrid::State::STATE_FLAGGED) {
        return;
    }

    // NOTE: should not reach to any mine in OpenRecursive
    if (this->mine_map[y][x] < 0) {
        std::cerr << "MineGame::OpenRecursive(x=" << x << ", y=" << y << "): run time error" << std::endl;
        return;
    }

    // adjust flag_count and remaining count
    if (this->GetGridState(x, y) == MineGameGrid::State::STATE_FLAGGED) {
        this->flag_count -= 1;
    }

    this->remaining_count -= 1;

    // NOTE: update grid based on mine count
    // we use this conversion trick because MineGame::State declaration is carefully arranged
    this->SetGridState(x, y, (MineGameGrid::State)this->mine_map[y][x]);

    // expand if we click on a 0
    if (this->mine_map[y][x] == 0) {
        int up = y - 1, down = y + 1, left = x - 1, right = x + 1;

        this->OpenRecursive(left, up);
        this->OpenRecursive(left, y);
        this->OpenRecursive(left, down);
        this->OpenRecursive(x, up);
        this->OpenRecursive(x, down);
        this->OpenRecursive(right, up);
        this->OpenRecursive(right, y);
        this->OpenRecursive(right, down);
    }
}


bool MineGame::IsValidPoint(int x, int y)
{
    if (0 <= x && x < this->width && 0 <= y && y < this->height) {
        return true;
    } else {
        return false;
    }
}

bool MineGame::HasMine(int x, int y)
{
    if (this->IsValidPoint(x, y)) {
        if (this->mine_map[y][x] == -1) {
            return true;
        }
    }

    return false;
}

bool MineGame::IsDirtyGrid(int x, int y) 
{
    if (this->IsValidPoint(x, y)) {
        if (this->grid_dirty_map[y][x] == 1) {
            return true;
        }
    }

    return false;
}

void MineGame::SetGridState(int x, int y, MineGameGrid::State state)
{
    if (this->grid_state_map[y][x] != state) {
        this->grid_state_map[y][x] = state;
        this->grid_dirty_map[y][x] = 1;
    }
}

////////////////////////////////////////////////////////////////////////////////////
MineGameGrid::MineGameGrid()
{
    this->state = MineGameGrid::State::STATE_COVERED;
    this->x = 0;
    this->y = 0;
}

MineGameGrid::~MineGameGrid()
{

}
