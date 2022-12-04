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

    if (this->IsValidPoint(x, y)) {
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
    std::cout << "Open(x=" << x << ", y=" << y << ")" << std::endl;

    // lazy init
    if (!this->init) {
        this->InitMines(x, y);
        this->init = true;

        // this is for debugging
        for (int y = 0; y < this->height; y++) {
            for (int x = 0; x < this->width; x++) {
                std::cout << std::setw(4) << this->mine_map[y][x];
            }

            std::cout << std::endl;
        }
    }

    if (this->IsValidPoint(x, y)) {
        if (this->GetState(x, y) == STATE_COVERED) {
            this->OpenRecursive(x, y, events);
        }
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
    if (this->IsValidPoint(x, y)) {
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

MineGameState MineGame::UpdateState(int x, int y)
{
    MineGameState state;

    switch (this->mine_map[y][x]) {
    case -1:
        state = MineGameState::STATE_MINE_OPEN;
        break;

    case 0:
        state = MineGameState::STATE_MINE_0;
        break;

    case 1:
        state = MineGameState::STATE_MINE_1;
        break;

    case 2:
        state = MineGameState::STATE_MINE_2;
        break;

    case 3:
        state = MineGameState::STATE_MINE_3;
        break;

    case 4:
        state = MineGameState::STATE_MINE_4;
        break;

    case 5:
        state = MineGameState::STATE_MINE_5;
        break;

    case 6:
        state = MineGameState::STATE_MINE_6;
        break;

    case 7:
        state = MineGameState::STATE_MINE_7;
        break;

    case 8:
        state = MineGameState::STATE_MINE_8;
        break;

    default:
        state = MineGameState::STATE_COVERED;
        break;
    }

    this->state_map[y][x] = state;

    return state;
}

void MineGame::ShowAllMines(std::vector<MineGameEvent> &events)
{
    // show all uncovered mines
    for (int y = 0; y < this->height; y++) {
        for (int x = 0; x < this->width; x++) {
            if (this->HasMine(x, y) && this->GetState(x, y) == MineGameState::STATE_COVERED) {
                MineGameEvent e;

                e.state = MineGameState::STATE_MINE_OPEN;
                e.x = x;
                e.y = y;

                events.push_back(e);
            }
        }
    }
}

void MineGame::OpenRecursive(int x, int y, std::vector<MineGameEvent> &events)
{
    if (this->state_map[y][x] == MineGameState::STATE_COVERED || this->state_map[y][x] == MineGameState::STATE_FLAGGED) {
        // case 1: explode on site
        if (this->mine_map[y][x] < 0) {
                this->state_map[y][x] = MineGameState::STATE_MINE_EXPLODE;

                MineGameEvent e;
                e.state = MineGameState::STATE_MINE_EXPLODE;
                e.x = x;
                e.y = y;
                events.push_back(e);

                this->ShowAllMines(events);            
        }
        // case 2: we stop at mine_N
        else if (this->mine_map[y][x] > 0) {
            MineGameEvent e;

            e.state = this->UpdateState(x, y);
            e.x = x;
            e.y = y;

            events.push_back(e);
        } 
        // case 3: recursively open neighbors
        else if (this->mine_map[y][x] == 0) {
            MineGameEvent e;

            this->state_map[y][x] = MineGameState::STATE_MINE_0;
            e.state = MineGameState::STATE_MINE_0;
            e.x = x;
            e.y = y;

            events.push_back(e);

            int up = y - 1, down = y + 1, left = x - 1, right = x + 1;

            if (this->IsValidPoint(left, up)) this->OpenRecursive(left, up, events);
            if (this->IsValidPoint(left, y)) this->OpenRecursive(left, y, events);
            if (this->IsValidPoint(left, down)) this->OpenRecursive(left, down, events);
            if (this->IsValidPoint(x, up)) this->OpenRecursive(x, up, events);
            if (this->IsValidPoint(x, down)) this->OpenRecursive(x, down, events);
            if (this->IsValidPoint(right, up)) this->OpenRecursive(right, up, events);
            if (this->IsValidPoint(right, y)) this->OpenRecursive(right, y, events);
            if (this->IsValidPoint(right, down)) this->OpenRecursive(right, down, events);
        }
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
