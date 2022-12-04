#include <iostream>
#include "game.h"

MineGame::MineGame()
{
    this->width = 0;
    this->height = 0;
    this->mine_map = NULL;
    this->flag_map = NULL;
    this->mine_count = 0;
    this->flag_count = 0;

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

    this->FreeMap();
    this->AllocateMap(width, height);
    this->InitMines(mine_count);
}

int MineGame::PlaceFlag(int x, int y)
{
    if (this->flag_map[y][x] != 0) {
        return 0;
    } else {
        this->flag_map[y][x] = 1;
        this->flag_count += 1;
        return 1;
    }
}

int MineGame::RemoveFlag(int x, int y)
{
    if (this->flag_map[y][x] == 0) {
        return 0;
    } else {
        this->flag_map[y][x] = 1;
        this->flag_count -= 1;
        return 1;
    }
}

int MineGame::Flip(int x, int y)
{
    return 0;
}

int MineGame::Peek(int x, int y)
{
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////

void MineGame::InitMines(int mine_count)
{
    // clean up maps
    for (int i = 0; i < this->height; i++) {
        for (int j = 0; j < this->width; j++) {
            this->mine_map[i][j] = 0;
            this->flag_map[i][j] = 0;
        }
    }

    // place mines randomly
    this->mine_count = 0;

    // calculate neighbors
}

int MineGame::AllocateMap(int width, int height)
{
    if (width > 0 && height > 0) {
        // allocate mine map and flag map
        this->mine_map = new int* [height];
        this->flag_map = new int* [height];
        for (int i = 0; i < height; i++) {
            this->mine_map[i] = new int [width];
            this->flag_map[i] = new int [width];
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
    if (this->flag_map != NULL) {
        for (int i = 0; i < this->height; i++) {
            delete [] this->flag_map[i];
        }

        delete [] this->flag_map;
        this->flag_map = NULL;
    }

    this->width = 0;
    this->height = 0;
}
