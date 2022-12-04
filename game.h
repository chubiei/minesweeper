#include <iostream>
#include <vector>

#ifndef __MINE_GAME_H__
#define __MINE_GAME_H__

enum MineGameState {
    GAME_READY,
    GAME_RUNNING, 
    GAME_WON,
    GAME_LOST
};

enum MineGameGridState {
    STATE_COVERED, 
    STATE_FLAGGED,
    STATE_FLAGGED_WRONG, 
    STATE_MINE_EXPLODE, 
    STATE_MINE_OPEN, 
    STATE_MINE_0,
    STATE_MINE_1,
    STATE_MINE_2,
    STATE_MINE_3,
    STATE_MINE_4,
    STATE_MINE_5,
    STATE_MINE_6,
    STATE_MINE_7,
    STATE_MINE_8
};

class MineGameEvent {
    public:
        MineGameGridState state;
        int x;
        int y;

    public:
        MineGameEvent();
        ~MineGameEvent();
};

class MineGame {
    public:

    public:
        MineGame();
        ~MineGame();

        int GetWidth() const { return this->width; }
        int GetHeight() const { return this->height; }
        int GetMineCount() const { return this->mine_count; }
        int GetFlagCount() const { return this->flag_count; }

        void SetBeginner();
        void SetIntermediate();
        void SetExpert();
        void SetCustom(int width, int height, int mine_count);
        void Reset();

        MineGameState GetGameState();
        MineGameGridState GetGridState(int x, int y);
        void TouchFlag(int x, int y, std::vector<MineGameEvent> &events);
        void Open(int x, int y, std::vector<MineGameEvent> &events);
        void OpenFast(int x, int y, std::vector<MineGameEvent> &events);

    private:
        void InitMines(int skip_x, int skip_y);
        bool HasMine(int x, int y);
        int AllocateMap(int width, int height);
        void FreeMap();

        void UpdateStateDirectly(int x, int y, MineGameGridState state);
        MineGameGridState UpdateStateAutomatically(int x, int y);
        
        void ShowAllMines(std::vector<MineGameEvent> &events);
        void OpenRecursive(int x, int y, std::vector<MineGameEvent> &events);
        bool IsValidPoint(int x, int y);

    private:
        int width;
        int height;
        int mine_count;
        int flag_count;
        int remaining_count;
        // -2: unknown, -1: mine, 0: no mines, 1: 1 mine, 2: 2 mines, etc...
        int **mine_map;

        // represent user interface state
        MineGameGridState **state_map;
        MineGameState game_state;
};



#endif
