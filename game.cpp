#include <iostream>
#include <iomanip>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include "game.h"

MineGame::MineGame()
{
    this->width = 0;
    this->height = 0;
    this->mine_map = NULL;
    this->state_map = NULL;
    this->mine_count = 0;
    this->flag_count = 0;
    this->init = false;

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

    // clean up map
    this->init = false;

    for (int i = 0; i < this->height; i++) {
        for (int j = 0; j < this->width; j++) {
            this->mine_map[i][j] = 0;
            this->state_map[i][j] = MineGameState::STATE_COVERED;
        }
    }
}

MineGameState MineGame::GetState(int x, int y)
{
    return this->state_map[y][x];
}

void MineGame::TouchFlag(int x, int y, std::vector<MineGameEvent> &events)
{
    MineGameEvent e;

    std::cout << "MineGame::TouchFlag(x=" << x << ", y=" << y << ")" << std::endl;

    if (0 <= x && x < this->width && 0 <= y && y < this->height) {
        if (this->GetState(x, y) == STATE_COVERED) {
            this->state_map[y][x] = STATE_FLAGGED;
            this->flag_count += 1;

            e.state = STATE_FLAGGED;
            e.x = x;
            e.y = y;
            events.push_back(e);
        } else if (this->GetState(x, y) == STATE_FLAGGED) {
            this->state_map[y][x] = STATE_COVERED;
            this->flag_count -= 1;

            e.state = STATE_COVERED;
            e.x = x;
            e.y = y;
            events.push_back(e);
        }
    }
}

void MineGame::Open(int x, int y, std::vector<MineGameEvent> &events)
{
    if (!this->init) {
        this->InitMines(x, y);
        this->init = true;
    }

    std::cout << "Open(x=" << x << ", y=" << y << ")" << std::endl;

    for (int y = 0; y < this->height; y++) {
        for (int x = 0; x < this->width; x++) {
            std::cout << std::setw(4) << this->mine_map[y][x];
        }

        std::cout << std::endl;
    }
}

void MineGame::OpenFast(int x, int y, std::vector<MineGameEvent> &events)
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

            this->state_map[y][x] = MineGameState::STATE_COVERED;
        }
    }
}

bool MineGame::HasMine(int x, int y)
{
    if (0 <= x && x < this->width && 0 <= y && y < this->height) {
        if (this->mine_map[y][x] == -1) {
            return true;
        }
    }

    return false;
}

int MineGame::AllocateMap(int width, int height)
{
    if (width > 0 && height > 0) {
        // allocate mine map and flag map
        this->mine_map = new int* [height];
        this->state_map = new MineGameState* [height];
        for (int i = 0; i < height; i++) {
            this->mine_map[i] = new int [width];
            this->state_map[i] = new MineGameState [width];
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
    if (this->state_map != NULL) {
        for (int i = 0; i < this->height; i++) {
            delete [] this->state_map[i];
        }

        delete [] this->state_map;
        this->state_map = NULL;
    }

    this->width = 0;
    this->height = 0;
}


////////////////////////////////////////////////////////////////////////////////////
MineGameEvent::MineGameEvent()
{
    this->state = MineGameState::STATE_COVERED;
    this->x = 0;
    this->y = 0;
}

MineGameEvent::~MineGameEvent()
{

}
