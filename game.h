#include <iostream>
#include <vector>

#ifndef __MINE_GAME_H__
#define __MINE_GAME_H__

class MineGameGrid {
    public:
        enum State {
            STATE_MINE_0 = 0,
            STATE_MINE_1 = 1,
            STATE_MINE_2 = 2,
            STATE_MINE_3 = 3,
            STATE_MINE_4 = 4,
            STATE_MINE_5 = 5,
            STATE_MINE_6 = 6,
            STATE_MINE_7 = 7,
            STATE_MINE_8 = 8,
            STATE_COVERED = 9, 
            STATE_FLAGGED = 10,
            STATE_FLAGGED_WRONG = 11, 
            STATE_MINE_EXPLODE = 12, 
            STATE_MINE_OPEN = 13
        };

    public:
        State state;
        int x;
        int y;

    public:
        MineGameGrid();
        ~MineGameGrid();
};

class MineGame {
    public:
        enum State {
            GAME_READY,
            GAME_RUNNING, 
            GAME_WON,
            GAME_LOST
        };

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

        State GetGameState();
        MineGameGrid::State GetGridState(int x, int y);
        void GetDirtyGrids(std::vector<MineGameGrid> &grids);
        void ClearDirtyGrids();

        void TouchFlag(int x, int y);
        void Open(int x, int y);
        void OpenFast(int x, int y);

    private:
        void InitMines(int skip_x, int skip_y);

        int AllocateMap(int width, int height);
        void FreeMap();

        void EndGame(int explode_x, int explode_y);
        void WinGame();
        void OpenRecursive(int x, int y);

        bool IsValidPoint(int x, int y);
        bool HasMine(int x, int y);

        bool IsDirtyGrid(int x, int y);
        void SetGridState(int x, int y, MineGameGrid::State state);

    private:
        int width;
        int height;
        int mine_count;
        int flag_count;
        int remaining_count;
        // -2: unknown, -1: mine, 0: no mines, 1: 1 mine, 2: 2 mines, etc...
        int **mine_map;

        // represent user interface state
        MineGameGrid::State **grid_state_map;
        int **grid_dirty_map;

        State game_state;
};



#endif
