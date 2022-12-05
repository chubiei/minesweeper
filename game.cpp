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
            this->state_map[i][j] = MineGameGrid::State::STATE_COVERED;
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
            this->state_map[i][j] = MineGameGrid::State::STATE_COVERED;
        }
    }
}

MineGame::State MineGame::GetGameState()
{
    return this->game_state;
}

MineGameGrid::State MineGame::GetGridState(int x, int y)
{
    return this->state_map[y][x];
}

void MineGame::TouchFlag(int x, int y, std::vector<MineGameGrid> &events)
{
    MineGameGrid e;

    std::cout << "MineGame::TouchFlag(x=" << x << ", y=" << y << ")" << std::endl;

    if (this->game_state == MineGame::State::GAME_RUNNING || this->game_state == MineGame::State::GAME_READY) {
        if (this->IsValidPoint(x, y)) {
            if (this->GetGridState(x, y) == MineGameGrid::State::STATE_COVERED) {
                this->state_map[y][x] = MineGameGrid::State::STATE_FLAGGED;
                this->flag_count += 1;

                e.state = MineGameGrid::State::STATE_FLAGGED;
                e.x = x;
                e.y = y;
                events.push_back(e);
            } else if (this->GetGridState(x, y) == MineGameGrid::State::STATE_FLAGGED) {
                this->state_map[y][x] = MineGameGrid::State::STATE_COVERED;
                this->flag_count -= 1;

                e.state = MineGameGrid::State::STATE_COVERED;
                e.x = x;
                e.y = y;
                events.push_back(e);
            }
        }
    }
}

void MineGame::Open(int x, int y, std::vector<MineGameGrid> &events)
{
    std::cout << "Open(x=" << x << ", y=" << y << ")" << std::endl;

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
        if (this->IsValidPoint(x, y)) {
            if (this->GetGridState(x, y) == MineGameGrid::State::STATE_COVERED) {
                this->OpenRecursive(x, y, events);

                // if game_state is changed inside OpenRecursive to GAME_WON, place all flags
                if (this->game_state == MineGame::State::GAME_WON) {
                    this->AppendAllFlags(events);
                }
            }
        }
    }
}

void MineGame::OpenFast(int x, int y, std::vector<MineGameGrid> &events)
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

            // NOTE: InitMines does not change state_map, because flags can be placed prior to init 
            // this->state_map[y][x] = MineGameGrid::State::STATE_COVERED;
        }
    }

    // NOTE: InitMines does not change flag_count, because flags can be placed prior to init 
    //this->flag_count = 0;
    this->remaining_count = this->width * this->height - this->mine_count;
    this->game_state = MineGame::State::GAME_RUNNING;
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
        this->state_map = new MineGameGrid::State* [height];
        for (int i = 0; i < height; i++) {
            this->mine_map[i] = new int [width];
            this->state_map[i] = new MineGameGrid::State [width];
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

void MineGame::UpdateStateDirectly(int x, int y, MineGameGrid::State state)
{
    if (this->state_map[y][x] == MineGameGrid::State::STATE_FLAGGED && state != MineGameGrid::State::STATE_FLAGGED) {
        this->flag_count -= 1;
    }

    this->state_map[y][x] = state;
    this->remaining_count -= 1;

    std::cout << "Flag count: " << this->flag_count << std::endl;
    std::cout << "Remaining count: " << this->remaining_count << std::endl;

    if (this->remaining_count == 0) {
        this->game_state = MineGame::State::GAME_WON;
        std::cout << "You won!!!" << std::endl;
    }
}

MineGameGrid::State MineGame::UpdateStateAutomatically(int x, int y)
{
    MineGameGrid::State state;

    switch (this->mine_map[y][x]) {
    case -1:
        state = MineGameGrid::State::STATE_MINE_OPEN;
        break;

    case 0:
        state = MineGameGrid::State::STATE_MINE_0;
        break;

    case 1:
        state = MineGameGrid::State::STATE_MINE_1;
        break;

    case 2:
        state = MineGameGrid::State::STATE_MINE_2;
        break;

    case 3:
        state = MineGameGrid::State::STATE_MINE_3;
        break;

    case 4:
        state = MineGameGrid::State::STATE_MINE_4;
        break;

    case 5:
        state = MineGameGrid::State::STATE_MINE_5;
        break;

    case 6:
        state = MineGameGrid::State::STATE_MINE_6;
        break;

    case 7:
        state = MineGameGrid::State::STATE_MINE_7;
        break;

    case 8:
        state = MineGameGrid::State::STATE_MINE_8;
        break;

    default:
        state = MineGameGrid::State::STATE_COVERED;
        break;
    }

    this->UpdateStateDirectly(x, y, state);

    return state;
}

void MineGame::ShowAllMines(std::vector<MineGameGrid> &events)
{
    // show all uncovered mines
    for (int y = 0; y < this->height; y++) {
        for (int x = 0; x < this->width; x++) {
            if (this->HasMine(x, y) && this->GetGridState(x, y) == MineGameGrid::State::STATE_COVERED) {
                MineGameGrid e;

                e.state = MineGameGrid::State::STATE_MINE_OPEN;
                e.x = x;
                e.y = y;

                events.push_back(e);
            }
        }
    }
}

void MineGame::OpenRecursive(int x, int y, std::vector<MineGameGrid> &events)
{
    if (this->state_map[y][x] == MineGameGrid::State::STATE_COVERED || this->state_map[y][x] == MineGameGrid::State::STATE_FLAGGED) {
        // case 1: explode on site
        if (this->mine_map[y][x] < 0) {
                MineGameGrid e;
                e.state = MineGameGrid::State::STATE_MINE_EXPLODE;
                e.x = x;
                e.y = y;
                events.push_back(e);

                this->UpdateStateDirectly(x, y, MineGameGrid::State::STATE_MINE_EXPLODE);
                this->ShowAllMines(events);

                this->game_state = MineGame::State::GAME_LOST;
                std::cout << "You've lost" << std::endl;
        }
        // case 2: we stop at mine_N
        else if (this->mine_map[y][x] > 0) {
            MineGameGrid e;

            e.state = this->UpdateStateAutomatically(x, y);
            e.x = x;
            e.y = y;

            events.push_back(e);
        } 
        // case 3: recursively open neighbors
        else if (this->mine_map[y][x] == 0) {
            MineGameGrid e;

            e.state = this->UpdateStateAutomatically(x, y);
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

void MineGame::AppendAllFlags(std::vector<MineGameGrid> &events)
{
    // show all uncovered flags
    for (int y = 0; y < this->height; y++) {
        for (int x = 0; x < this->width; x++) {
            if (this->HasMine(x, y) && this->GetGridState(x, y) == MineGameGrid::State::STATE_COVERED) {
                MineGameGrid e;

                e.state = MineGameGrid::State::STATE_FLAGGED;
                e.x = x;
                e.y = y;

                events.push_back(e);
            }
        }
    }

    this->flag_count = this->mine_count;
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
MineGameGrid::MineGameGrid()
{
    this->state = MineGameGrid::State::STATE_COVERED;
    this->x = 0;
    this->y = 0;
}

MineGameGrid::~MineGameGrid()
{

}
