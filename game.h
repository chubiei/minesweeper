#include <iostream>

#ifndef __MINE_GAME_H__
#define __MINE_GAME_H__

class MineGame {
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

        int PlaceFlag(int x, int y);
        int RemoveFlag(int x, int y);
        int Flip(int x, int y);
        int Peek(int x, int y);

    private:
        void InitMines(int mine_count);
        int AllocateMap(int width, int height);
        void FreeMap();

    private:
        int width;
        int height;
        int mine_count;
        int flag_count;
        int **mine_map;
        int **flag_map;
};


#endif
