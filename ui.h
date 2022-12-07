#include "sdl_headers.h"
#include "game.h"

#ifndef __MINE_UI_H__
#define __MINE_UI_H__

class CounterUI;
class FaceButtonUI;
class MineGridUI;
class SplashScreen;

class MineGameTimer {
    public:
        MineGameTimer();
        ~MineGameTimer();

        int Add(Uint32 millisecond);
        void Remove();

        SDL_TimerID GetId() const;
        Uint32 GetEventId() const;
        Uint32 GetInterval() const;
        Uint32 GetTickCount() const;
        Uint64 GetStartTick() const;

    private:
        static Uint32 TimerCallback(Uint32 interval, void *param);

    private:
        Uint32 interval;
        SDL_TimerID timer_id;
        Uint32 event_id;
        Uint32 tick_count;
        Uint64 start_tick;
};

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
        void GameOpen(int x, int y);
        void GameTouchFlag(int x, int y);
        void GameReset();
        void GameGetDirtyGrids(std::vector<MineGameGrid> &grids);

    private:
        int CreateWindow();
        void DestroyWindow();

        int ResizeWindow();
        int RedrawWindow();
        int RefreshWindow();

        int DispatchEvent(SDL_Event *e);
        int HandleTimerEvent(SDL_UserEvent *e);

        void ShowWinningSplash();
        void StopWinningSplash();

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
        SplashScreen *winning_splash;

        // splash timer
        MineGameTimer *splash_timer;

        // count down timer
        MineGameTimer *count_down_timer;

        // game
        MineGame *game;
};

class SplashScreen {
    public:
        SplashScreen(MineGameWindowUI *window);
        ~SplashScreen();

        void SetLocation(int x, int y);

        int LoadResources();
        void ReleaseResources();

        void Show();
        void Update(Uint64 elapse_time_ms);
        void Hide();

    private:
        // owner window
        MineGameWindowUI *window;
        Uint8 alpha;
        SDL_Texture *texture;
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

        // event handlers
        int HandleMouseMotionEvent(SDL_MouseMotionEvent *event);
        int HandleMouseButtonEvent(SDL_MouseButtonEvent *event);
        int HandleUserEvent(SDL_UserEvent *event);

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
        int RedrawDirtyGrids();

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
