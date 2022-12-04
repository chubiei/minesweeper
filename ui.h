#include "sdl_headers.h"
#include "game.h"

#ifndef __MINE_UI_H__
#define __MINE_UI_H__

class CounterUI;
class FaceButtonUI;
class MineGridUI;

class MineGameWindowUI {
    public:
        MineGameWindowUI(MineGame *game);
        ~MineGameWindowUI();

        int CreateComponents();
        void DestroyComponents();
        int ProcessEvents();

        // helper function for components
        SDL_Texture *LoadTextureFromFile(const char *path);
        SDL_Texture *CreateTexture(int width, int height);
        int UpdateWindowTexture(SDL_Texture *texture, const SDL_Rect *rect);
        int UpdateTexture(SDL_Texture *updated_texture, SDL_Texture *texture, const SDL_Rect *rect);
        void GameOpen(int x, int y, std::vector<MineGameEvent> &events);
        void GameTouchFlag(int x, int y, std::vector<MineGameEvent> &events);
        void GameReset();

    private:
        int CreateWindow();
        void DestroyWindow();

        int ResizeWindow();
        int RefreshWindow();

        int DispatchEvent(SDL_Event *e);

    private:
        // GUI for window
        SDL_Window *window;
        SDL_Renderer *renderer;
        SDL_Texture *window_texture;
        bool window_texture_dirty;

        // components
        MineGridUI *mine_grid;
        FaceButtonUI *face_button;
        CounterUI *mine_counter;
        CounterUI *time_counter;

        // game
        MineGame *game;
};


class CounterUI {
    public:
        CounterUI(MineGameWindowUI *window);
        ~CounterUI();

        void SetLocation(int x, int y);
        const SDL_Rect *GetRect() const;
        int GetWidth() const;
        int GetHeight() const;

        void SetCount(int c);
        int GetCount() const;
        void IncreaseCount();

        int LoadResources();
        void ReleaseResources();

        int Redraw();

        // timer functions
        int AddTimer(int second);
        void RemoveTimer();

        // event handlers
        int HandleMouseMotionEvent(SDL_MouseMotionEvent *event);
        int HandleMouseButtonEvent(SDL_MouseButtonEvent *event);
        int HandleUserEvent(SDL_UserEvent *event);

    private:
        SDL_TimerID GetTimerId() const;
        Uint32 GetTimerEventId() const;
        Uint32 GetTimerInterval() const;
    
        static Uint32 TimerCallback(Uint32 interval, void *param);

    private:
        // owner window
        MineGameWindowUI *window;
    
        // digits
        SDL_Texture **digit;
        SDL_Texture *background;

        // component rect
        SDL_Rect *background_rect;
        SDL_Rect *digit1_rect;
        SDL_Rect *digit2_rect;
        SDL_Rect *digit3_rect;

        // digit to display
        int count;
        int digit1;
        int digit2;
        int digit3;

        // timer
        Uint32 timer_interval;
        SDL_TimerID timer_id;
        Uint32 timer_event_id;
};

class FaceButtonUI {
    public:
        enum Status {
            STATUS_FACE_PRESSED = 0,
            STATUS_FACE_UNPRESSED = 1,
            STATUS_FACE_WIN = 2,
            STATUS_FACE_LOSE = 3
        };

    public:
        FaceButtonUI(MineGameWindowUI *window);
        ~FaceButtonUI();

        void SetLocation(int x, int y);
        const SDL_Rect *GetRect() const;
        int GetWidth() const;
        int GetHeight() const;

        void SetStatus(Status status);
        Status GetStatus() const;

        int LoadResources();
        void ReleaseResources();

        int Redraw();

        // event handlers
        int HandleMouseMotionEvent(SDL_MouseMotionEvent *event);
        int HandleMouseButtonEvent(SDL_MouseButtonEvent *event);

    private:
        // owner window
        MineGameWindowUI *window;

        // faces
        SDL_Texture *face_pressed;
        SDL_Texture *face_unpressed;
        SDL_Texture *face_win;
        SDL_Texture *face_lose;

        // component rect
        SDL_Rect *rect;

        // button status
        Status current_status;
};

class MineGridUI {
    public:
        MineGridUI(MineGameWindowUI *window);
        ~MineGridUI();

        void SetLocation(int x, int y);
        void SetGameSize(int x, int y);
        const SDL_Rect *GetRect() const;
        int GetWidth() const;
        int GetHeight() const;

        int LoadResources();
        void ReleaseResources();

        int Redraw();

        // event handlers
        int HandleMouseMotionEvent(SDL_MouseMotionEvent *event);
        int HandleMouseButtonEvent(SDL_MouseButtonEvent *event);

    private:
        int InitTexture(); 
        int HandleGameEvents(const std::vector<MineGameEvent> &events);

    private:
        // owner window
        MineGameWindowUI *window;

        // mines
        SDL_Texture **mine;
        SDL_Texture *mine_covered;
        SDL_Texture *mine_flagged;
        SDL_Texture *mine_flagged_wrong;
        SDL_Texture *mine_open_black;
        SDL_Texture *mine_open_red;

        // grid size
        int game_x, game_y;

        // component texture
        SDL_Texture *grid_texture;

        // component rect
        SDL_Rect *rect;
};

#endif
