#include <iostream>

#ifndef __MINE_GAME_H__
#define __MINE_GAME_H__

class MineGame {
    public:
        enum State {
            STATE_EMPTY, 
            STATE_FLAGGED,
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

        void TouchFlag(int x, int y);
        void Open(int x, int y);
        void OpenFast(int x, int y);

    private:
        void InitMines(int skip_x, int skip_y);
        bool HasMine(int x, int y);
        int AllocateMap(int width, int height);
        void FreeMap();

    private:
        int width;
        int height;
        int mine_count;
        int flag_count;
        // -2: unknown, -1: mine, 0: no mines, 1: 1 mine, 2: 2 mines, etc...
        int **mine_map;

        // represent user interface state
        int **state_map;
        bool init;
};


#endif
