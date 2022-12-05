#include <iostream>
#include "ui.h"
#include "game.h"

#define WINDOWS_EDGE_MARGIN 10

#define COUNTER_EDGE_MARGIN 2
#define COUNTER_DIGIT_MARGIN 1
#define COUNTER_DIGIT_WIDTH 19
#define COUNTER_DIGIT_HEIGHT 40

#define FACE_BUTTON_EDGE_MARGIN 2
#define FACE_BUTTON_SIZE 40

#define MINE_GRID_EDGE_MARGIN 2
#define MINE_GRID_MINE_SIZE 24


////////////////////////////////////////////////////////////////////////////////////
MineGameWindowUI::MineGameWindowUI(MineGame *game)
{
    this->window = NULL;
    this->renderer = NULL;
    this->window_texture = NULL;
    this->window_texture_dirty = true;

    this->mine_grid = NULL;
    this->face_button = NULL;
    this->mine_counter = NULL;
    this->time_counter = NULL;

    this->game = game;
}

MineGameWindowUI::~MineGameWindowUI()
{
    this->DestroyComponents();
}

int MineGameWindowUI::CreateComponents()
{
    this->CreateWindow();

    this->time_counter = new CounterUI(this);
    this->time_counter->LoadResources();
    this->time_counter->SetCount(0);

    this->mine_counter = new CounterUI(this);
    this->mine_counter->LoadResources();
    this->mine_counter->SetCount(this->game->GetMineCount());

    this->face_button = new FaceButtonUI(this);
    this->face_button->LoadResources();

    this->mine_grid = new MineGridUI(this);
    this->mine_grid->LoadResources();
    this->mine_grid->SetGameSize(this->game->GetWidth(), this->game->GetHeight());

    this->ResizeWindow();
    // show window 
    //SDL_ShowWindow(this->window);

    return 0;
}

void MineGameWindowUI::DestroyComponents()
{
    if (this->mine_grid != NULL) {
        delete this->mine_grid;
        this->mine_grid = NULL;
    }

    if (this->face_button != NULL) {
        delete this->face_button;
        this->face_button = NULL;
    }

    if (this->mine_counter != NULL) {
        delete this->mine_counter;
        this->mine_counter = NULL;
    }

    if (this->time_counter != NULL) {
        delete this->time_counter;
        this->time_counter = NULL;
    }

    this->DestroyWindow();
}

int MineGameWindowUI::ProcessEvents()
{
    while (true) {
        SDL_Event e; 

        if (SDL_WaitEvent(&e) <= 0) {
            std::cerr << "SDL_WaitEvent failed with error: " << SDL_GetError() << std::endl;
            continue;
        }

        if (e.type == SDL_QUIT) {
            std::cout << "got quit event" << std::endl;
            break;
        }

        this->DispatchEvent(&e);
        this->RefreshWindow();
    }

    return 0;
}

int MineGameWindowUI::DispatchEvent(SDL_Event *base_event)
{
    MineGame::State old_state = this->game->GetGameState();
    int old_flag_count = this->game->GetFlagCount();

    // mouse motion
    if (base_event->type == SDL_MOUSEMOTION) {
        SDL_MouseMotionEvent *e = (SDL_MouseMotionEvent*)base_event;

        //this->mine_counter->HandleMouseMotionEvent(e);
        //this->time_counter->HandleMouseMotionEvent(e);
        this->face_button->HandleMouseMotionEvent(e);
        this->mine_grid->HandleMouseMotionEvent(e);
    }
    // mouse click
    else if (base_event->type == SDL_MOUSEBUTTONUP || base_event->type == SDL_MOUSEBUTTONDOWN) {
        SDL_MouseButtonEvent *e = (SDL_MouseButtonEvent*)base_event;

        //this->mine_counter->HandleMouseButtonEvent(e);
        //this->time_counter->HandleMouseButtonEvent(e);
        this->face_button->HandleMouseButtonEvent(e);
        this->mine_grid->HandleMouseButtonEvent(e);
    }
    // user defined events
    else if (SDL_USEREVENT <= base_event->type && base_event->type < SDL_LASTEVENT) {
        SDL_UserEvent *e = (SDL_UserEvent*)base_event;

        //this->mine_counter->HandleUserEvent(e);
        this->time_counter->HandleUserEvent(e);

    }    

    MineGame::State new_state = this->game->GetGameState();
    int new_flag_count = this->game->GetFlagCount();
    
    if (old_state == MineGame::State::GAME_READY && new_state == MineGame::State::GAME_RUNNING) {
        this->time_counter->AddTimer(1);
    } else if (new_state == MineGame::State::GAME_WON) {
        this->face_button->SetStatus(FaceButtonUI::STATUS_FACE_WIN);
        this->time_counter->RemoveTimer();
    } else if (new_state == MineGame::State::GAME_LOST) {
        this->face_button->SetStatus(FaceButtonUI::STATUS_FACE_LOSE);
        this->time_counter->RemoveTimer();
    }

    if (old_flag_count != new_flag_count) {
        this->mine_counter->SetCount(this->game->GetMineCount() - new_flag_count);
        this->mine_counter->Redraw();
    }

    return 0;
}

SDL_Texture* MineGameWindowUI::LoadTextureFromFile(const char *path)
{
    SDL_RWops *rw;
    SDL_Surface *surface;
    SDL_Texture *texture;

    std::cout << "Loading " << path << "..." << std::endl;

    rw = SDL_RWFromFile(path, "r");
    if (rw == NULL) {
        std::cerr << "SDL_RWFromFile failed with error: " << SDL_GetError() << std::endl;
        return NULL;
    }

    surface = IMG_Load_RW(rw, 0);
    if (surface == NULL) {
        std::cerr << "IMG_Load_RW failed with error: " << SDL_GetError() << std::endl;
        SDL_RWclose(rw);
        return NULL;
    }

    texture = SDL_CreateTextureFromSurface(this->renderer, surface);
    if (texture == NULL) {
        std::cerr << "SDL_CreateTextureFromSurface failed with error: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(surface);
        SDL_RWclose(rw);
        return NULL;
    }

    //SDL_SetTextureAlphaMod(texture, 0);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_NONE);

    SDL_FreeSurface(surface);
    SDL_RWclose(rw);

    return texture;
}

SDL_Texture *MineGameWindowUI::CreateTexture(int width, int height)
{
    SDL_Texture *texture;

    texture = SDL_CreateTexture(this->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, width, height);
    //new_texture = SDL_CreateTexture(new_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, screen_width, screen_height);
    if (texture == NULL) {
        std::cerr << "SDL_CreateTexture failed with error: " << SDL_GetError() << std::endl;
        return NULL;
    }

    return texture;
}

void MineGameWindowUI::GameOpen(int x, int y)
{
    this->game->Open(x, y);
}

void MineGameWindowUI::GameTouchFlag(int x, int y)
{
    this->game->TouchFlag(x, y);
}

void MineGameWindowUI::GameReset()
{
    this->game->Reset();

    this->mine_grid->SetGameSize(this->game->GetWidth(), this->game->GetHeight());
    this->mine_grid->Redraw();

    this->time_counter->SetCount(0);
    this->time_counter->Redraw();
    this->time_counter->RemoveTimer();

    this->mine_counter->SetCount(this->game->GetFlagCount());
    this->mine_counter->Redraw();
}

void MineGameWindowUI::GameGetDirtyGrids(std::vector<MineGameGrid> &grids)
{
    this->game->GetDirtyGrids(grids);
    this->game->ClearDirtyGrids();
}

////////////////////////////////////////////////////////////////////////////////////
int MineGameWindowUI::CreateWindow()
{
    int width = 10, height = 10;
    SDL_Window *new_window;
    SDL_Renderer *new_renderer;
    SDL_Texture *new_texture;

    new_window = SDL_CreateWindow("Minesweeper", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
    //new_window = SDL_CreateWindow("Minesweeper", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (new_window == NULL) {
        std::cerr << "SDL_CreateWindowAndRenderer failed with error: " << SDL_GetError() << std::endl;
        return -1;
    }

    //new_renderer = SDL_CreateRenderer(new_window, -1, SDL_RENDERER_TARGETTEXTURE | SDL_RENDERER_SOFTWARE);
    //new_renderer = SDL_CreateRenderer(new_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    new_renderer = SDL_CreateRenderer(new_window, -1, SDL_RENDERER_TARGETTEXTURE);
    if (new_renderer == NULL) {
        std::cerr << "SDL_CreateWindowAndRenderer failed with error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(new_window);
        return -1;
    }

    this->window = new_window;
    this->renderer = new_renderer;

    new_texture = this->CreateTexture(width, height);
    if (new_texture == NULL) {
        SDL_DestroyRenderer(new_renderer);
        SDL_DestroyWindow(new_window);
        return -1;
    }

    this->window_texture = new_texture;

    return 0;
}

int MineGameWindowUI::ResizeWindow()
{
    int total_width, total_height;

    total_width = this->mine_grid->GetWidth() + WINDOWS_EDGE_MARGIN * 2;
    total_height = this->mine_grid->GetHeight() + this->mine_counter->GetHeight() + WINDOWS_EDGE_MARGIN * 3;

    // always get a new texture when resized
    if (true) {
        SDL_Texture *texture;

        texture = this->CreateTexture(total_width, total_height);
        if (texture == NULL) {
            return -1;
        }

        if (this->window_texture != NULL) {
            SDL_DestroyTexture(this->window_texture);
        }

        this->window_texture = texture;
    }

    // resize window and move components to the correct location
    SDL_ShowWindow(this->window);
    SDL_SetWindowSize(this->window, total_width, total_height);
    this->mine_grid->SetLocation(WINDOWS_EDGE_MARGIN, this->mine_counter->GetHeight() + WINDOWS_EDGE_MARGIN * 2);
    this->mine_counter->SetLocation(WINDOWS_EDGE_MARGIN, WINDOWS_EDGE_MARGIN);
    this->time_counter->SetLocation(total_width - WINDOWS_EDGE_MARGIN - this->time_counter->GetWidth(), WINDOWS_EDGE_MARGIN);
    this->face_button->SetLocation(((total_width - this->face_button->GetWidth()) / 2), WINDOWS_EDGE_MARGIN);

    // set background to gray
    SDL_SetRenderTarget(this->renderer, this->window_texture);
    SDL_RenderClear(this->renderer);
    SDL_SetRenderDrawColor(this->renderer, 0xC0, 0xC0, 0xC0, 0);
    SDL_RenderFillRect(this->renderer, NULL);

    // redraw components
    this->mine_grid->Redraw();
    this->time_counter->Redraw();
    this->mine_counter->Redraw();
    this->face_button->Redraw();

    this->RefreshWindow();

    return 0;
}

void MineGameWindowUI::DestroyWindow()
{
    if (this->window_texture != NULL) {
        SDL_DestroyTexture(this->window_texture);
        this->window_texture = NULL;
    }

    if (this->renderer != NULL) {
        SDL_DestroyRenderer(this->renderer);
        this->renderer = NULL;
    }

    if (this->window != NULL) {
        SDL_DestroyWindow(this->window);
        this->window = NULL;
    }
}

int MineGameWindowUI::UpdateWindowTexture(SDL_Texture *texture, const SDL_Rect *rect)
{
    if (this->window_texture != NULL) {
        SDL_SetRenderTarget(this->renderer, this->window_texture);
        SDL_RenderCopy(this->renderer, texture, NULL, rect);
        this->window_texture_dirty = true;
    }

    return 0;
}

int MineGameWindowUI::UpdateTexture(SDL_Texture *updated_texture, SDL_Texture *texture, const SDL_Rect *rect)
{
    SDL_SetRenderTarget(this->renderer, updated_texture);
    SDL_RenderCopy(this->renderer, texture, NULL, rect);

    return 0;
}

int MineGameWindowUI::RefreshWindow()
{
    // update texture to window
    if (this->window_texture != NULL && this->window_texture_dirty) {
        std::cout << "RefreshWindow" << std::endl;

        SDL_SetRenderTarget(this->renderer, NULL);
        SDL_RenderCopy(this->renderer, this->window_texture, NULL, NULL);
        SDL_RenderPresent(this->renderer);
        this->window_texture_dirty = false;
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////
CounterUI::CounterUI(MineGameWindowUI *window)
{
    this->window = window;

    this->digit = new SDL_Texture * [10];

    this->count = 0;
    this->digit1 = 0;
    this->digit2 = 0;
    this->digit3 = 0;

    this->background_rect = new SDL_Rect();
    this->digit1_rect = new SDL_Rect();
    this->digit2_rect = new SDL_Rect();
    this->digit3_rect = new SDL_Rect();

    this->background_rect->x = 0;
    this->background_rect->y = 0;
    this->background_rect->w = COUNTER_DIGIT_WIDTH * 3 + COUNTER_EDGE_MARGIN * 2 + COUNTER_DIGIT_MARGIN * 6;
    this->background_rect->h = COUNTER_DIGIT_HEIGHT + COUNTER_EDGE_MARGIN * 2 + COUNTER_DIGIT_MARGIN * 2;

    this->digit1_rect->x = 0;
    this->digit1_rect->y = 0;
    this->digit1_rect->w = COUNTER_DIGIT_WIDTH;
    this->digit1_rect->h = COUNTER_DIGIT_HEIGHT;

    this->digit2_rect->x = 0;
    this->digit2_rect->y = 0;
    this->digit2_rect->w = COUNTER_DIGIT_WIDTH;
    this->digit2_rect->h = COUNTER_DIGIT_HEIGHT;

    this->digit3_rect->x = 0;
    this->digit3_rect->y = 0;
    this->digit3_rect->w = COUNTER_DIGIT_WIDTH;
    this->digit3_rect->h = COUNTER_DIGIT_HEIGHT;

    this->timer_event_id = (Uint32)-1;
    this->timer_id = 0;
    this->timer_interval = 0;
}

CounterUI::~CounterUI()
{
    this->ReleaseResources();

    delete [] this->digit;
    delete this->background_rect;
    delete this->digit1_rect;
    delete this->digit2_rect;
    delete this->digit3_rect;
}

void CounterUI::SetLocation(int x, int y)
{
    this->background_rect->x = x;
    this->background_rect->y = y;

    this->digit1_rect->x = x + COUNTER_EDGE_MARGIN + COUNTER_DIGIT_MARGIN;
    this->digit1_rect->y = y + COUNTER_EDGE_MARGIN + COUNTER_DIGIT_MARGIN;

    this->digit2_rect->x = x + COUNTER_EDGE_MARGIN + COUNTER_DIGIT_WIDTH * 1 + COUNTER_DIGIT_MARGIN * 3;
    this->digit2_rect->y = y + COUNTER_EDGE_MARGIN + COUNTER_DIGIT_MARGIN;

    this->digit3_rect->x = x + COUNTER_EDGE_MARGIN + COUNTER_DIGIT_WIDTH * 2 + COUNTER_DIGIT_MARGIN * 5;
    this->digit3_rect->y = y + COUNTER_EDGE_MARGIN + COUNTER_DIGIT_MARGIN;
}

const SDL_Rect *CounterUI::GetRect() const
{
    return this->background_rect;
}

int CounterUI::GetWidth() const
{
    return this->GetRect()->w;
}

int CounterUI::GetHeight() const
{
    return this->GetRect()->h;
}

void CounterUI::SetCount(int c)
{
    this->count = c;

    if (c < 0) {
        this->digit1 = 0;
        this->digit2 = 0;
        this->digit3 = 0;
    } else if (c < 10) {
        this->digit1 = 0;
        this->digit2 = 0;
        this->digit3 = c;
    } else if (c < 100) {
        this->digit1 = 0;
        this->digit2 = c / 10;
        this->digit3 = c % 10;
    } else {
        this->digit1 = c / 100;
        this->digit2 = (c % 100) / 10;
        this->digit3 = c % 10;
    }

    //std::cout << "SetCount(" << this->count << ")" << std::endl;
    //std::cout << "digits = (" << this->digit1 << ", " << this->digit2 << ", " << this->digit3 << ")" << std::endl;
}

int CounterUI::GetCount() const
{
    return this->count;
}

void CounterUI::IncreaseCount()
{
    int c = this->GetCount();
    this->SetCount(c + 1);
}

int CounterUI::LoadResources()
{
    this->digit[0] = this->window->LoadTextureFromFile("./images/png/d0.png");
    this->digit[1] = this->window->LoadTextureFromFile("./images/png/d1.png");
    this->digit[2] = this->window->LoadTextureFromFile("./images/png/d2.png");
    this->digit[3] = this->window->LoadTextureFromFile("./images/png/d3.png");
    this->digit[4] = this->window->LoadTextureFromFile("./images/png/d4.png");
    this->digit[5] = this->window->LoadTextureFromFile("./images/png/d5.png");
    this->digit[6] = this->window->LoadTextureFromFile("./images/png/d6.png");
    this->digit[7] = this->window->LoadTextureFromFile("./images/png/d7.png");
    this->digit[8] = this->window->LoadTextureFromFile("./images/png/d8.png");
    this->digit[9] = this->window->LoadTextureFromFile("./images/png/d9.png");
    this->background = this->window->LoadTextureFromFile("./images/png/nums_background.png");

    return 0;
}

void CounterUI::ReleaseResources()
{
    for (int i = 0; i < 10; i++) {
        if (this->digit[i] != NULL) {
            SDL_DestroyTexture(this->digit[i]);
            this->digit[i] = NULL;
        }
    }

    if (this->background != NULL) {
        SDL_DestroyTexture(this->background);
        this->background = NULL;
    }

    this->RemoveTimer();
}

int CounterUI::Redraw()
{
    this->window->UpdateWindowTexture(this->background, this->background_rect);
    this->window->UpdateWindowTexture(this->digit[this->digit1], this->digit1_rect);
    this->window->UpdateWindowTexture(this->digit[this->digit2], this->digit2_rect);
    this->window->UpdateWindowTexture(this->digit[this->digit3], this->digit3_rect);

    return 0;
}

int CounterUI::AddTimer(int second)
{
    if (this->timer_event_id == (Uint32(-1))) {
        Uint32 eid;

        eid = SDL_RegisterEvents(1);
        if (eid == (Uint32)(-1)) {
            std::cerr << "SDL_RegisterEvents failed with error: " << SDL_GetError() << std::endl;
            return -1;
        }

        std::cout << "Register event id = " << eid << std::endl;
        this->timer_event_id = eid;
    }

    if (this->timer_interval == 0) {
        SDL_TimerID tid;
        Uint32 interval;

        interval = second * 1000;
        tid = SDL_AddTimer(interval, CounterUI::TimerCallback, this);
        if (tid == 0) {
            std::cerr << "SDL_AddTimer failed with error: " << SDL_GetError() << std::endl;
            return -1;
        }

        std::cout << "Add timer with id = " << tid << ", interval = " << interval << std::endl;
        this->timer_id = tid;
        this->timer_interval = interval;
    }

    return 0;
}

void CounterUI::RemoveTimer()
{
    if (this->timer_interval != 0) {
        SDL_RemoveTimer(this->timer_id);
        this->timer_id = 0;
        this->timer_interval = 0;
    }
}

int CounterUI::HandleMouseMotionEvent(SDL_MouseMotionEvent *event)
{
    // do nothing
    return 0;
}

int CounterUI::HandleMouseButtonEvent(SDL_MouseButtonEvent *event)
{
    // do nothing
    return 0;
}

int CounterUI::HandleUserEvent(SDL_UserEvent *event)
{
    if (event->type == this->GetTimerEventId()) {
        this->IncreaseCount();
        this->Redraw();
    }

    return 0;
}

SDL_TimerID CounterUI::GetTimerId() const
{
    return this->timer_id;
}

Uint32 CounterUI::GetTimerEventId() const
{
    return this->timer_event_id;
}

Uint32 CounterUI::GetTimerInterval() const
{
    return this->timer_interval;
}

Uint32 CounterUI::TimerCallback(Uint32 interval, void *param)
{
    SDL_Event event;
    CounterUI *counter;

    counter = (CounterUI*)param;

    SDL_memset(&event, 0, sizeof(event)); /* or SDL_zero(event) */
    event.type = counter->GetTimerEventId();
    event.user.code = 0;
    event.user.data1 = counter;
    event.user.data2 = 0;

    SDL_PushEvent(&event);

    return counter->GetTimerInterval();
}

////////////////////////////////////////////////////////////////////////////////////
FaceButtonUI::FaceButtonUI(MineGameWindowUI *window)
{
    this->window = window;

    this->face_pressed = NULL;
    this->face_unpressed = NULL;
    this->face_win = NULL;

    this->rect = new SDL_Rect();

    this->rect->x = 0;
    this->rect->y = 0;
    this->rect->w = FACE_BUTTON_SIZE + FACE_BUTTON_EDGE_MARGIN * 2;
    this->rect->h = FACE_BUTTON_SIZE + FACE_BUTTON_EDGE_MARGIN * 2;

    this->current_status = FaceButtonUI::STATUS_FACE_UNPRESSED;
}

FaceButtonUI::~FaceButtonUI()
{
    this->ReleaseResources();

    delete this->rect;
}


void FaceButtonUI::SetLocation(int x, int y)
{
    this->rect->x = x;
    this->rect->y = y;
}

const SDL_Rect *FaceButtonUI::GetRect() const
{
    return this->rect;
}

int FaceButtonUI::GetWidth() const
{
    return this->GetRect()->w;
}

int FaceButtonUI::GetHeight() const
{
    return this->GetRect()->h;
}

void FaceButtonUI::SetStatus(FaceButtonUI::Status status)
{
    if (this->current_status != status) {
        this->current_status = status;
        this->Redraw();
    }
}

FaceButtonUI::Status FaceButtonUI::GetStatus() const
{
    return this->current_status;
}

int FaceButtonUI::LoadResources()
{
    this->face_pressed = this->window->LoadTextureFromFile("./images/png/face_pressed.png");
    this->face_unpressed = this->window->LoadTextureFromFile("./images/png/face_unpressed.png");
    this->face_win = this->window->LoadTextureFromFile("./images/png/face_win.png");
    this->face_lose = this->window->LoadTextureFromFile("./images/png/face_lose.png");

    return 0;
}

void FaceButtonUI::ReleaseResources()
{
    if (this->face_pressed != NULL) {
        SDL_DestroyTexture(this->face_pressed);
        this->face_pressed = NULL;
    }

    if (this->face_unpressed != NULL) {
        SDL_DestroyTexture(this->face_unpressed);
        this->face_unpressed = NULL;
    }

    if (this->face_win != NULL) {
        SDL_DestroyTexture(this->face_win);
        this->face_win = NULL;
    }

    if (this->face_lose != NULL) {
        SDL_DestroyTexture(this->face_lose);
        this->face_lose = NULL;
    }
}

int FaceButtonUI::Redraw()
{
    // pressed
    if (this->current_status == STATUS_FACE_PRESSED) {
        this->window->UpdateWindowTexture(this->face_pressed, this->GetRect());
    }
    // unpressed
    else if (this->current_status == STATUS_FACE_UNPRESSED) {
        this->window->UpdateWindowTexture(this->face_unpressed, this->GetRect());
    }
    // win
    else if (this->current_status == STATUS_FACE_WIN) {
        this->window->UpdateWindowTexture(this->face_win, this->GetRect());
    }
    // lose
    else {
        this->window->UpdateWindowTexture(this->face_lose, this->GetRect());
    }

    return 0;
}

int FaceButtonUI::HandleMouseMotionEvent(SDL_MouseMotionEvent *event)
{
    int x1, x2, y1, y2, last_x, last_y;
    bool start_in, end_in;

    x1 = this->GetRect()->x;
    x2 = this->GetRect()->x + this->GetRect()->w;
    y1 = this->GetRect()->y;
    y2 = this->GetRect()->y + this->GetRect()->h;
    last_x = event->x - event->xrel;
    last_y = event->y - event->yrel;

    start_in = (x1 <= last_x && last_x < x2 && y1 <= last_y && last_y < y2);
    end_in = (x1 <= event->x && event->x < x2 && y1 <= event->y && event->y < y2);

    // leaving component
    if (start_in && !end_in) {
        /*
        std::cout << "SDL_MouseMotionEvent: ";
        std::cout << "type = SDL_MOUSEMOTION";
        std::cout << ", state = " << event->state;
        std::cout << ", x = " << event->x << ", y = " << event->y;
        std::cout << ", xrel = " << event->xrel << ", yrel = " << event->yrel;
        std::cout << ", type = leaving" << std::endl;
        */
        // handle leaving with click button hold
        if (this->GetStatus() == FaceButtonUI::STATUS_FACE_PRESSED) {
            this->SetStatus(FaceButtonUI::STATUS_FACE_UNPRESSED);
            this->window->GameReset();
        }
    }
    // entering component
    else if (!start_in && end_in) {
        /*
        std::cout << "SDL_MouseMotionEvent: ";
        std::cout << "type = SDL_MOUSEMOTION";
        std::cout << ", state = " << event->state;
        std::cout << ", x = " << event->x << ", y = " << event->y;
        std::cout << ", xrel = " << event->xrel << ", yrel = " << event->yrel;
        std::cout << ", type = entering" << std::endl;
        */
    } else {
        /*
        std::cout << "SDL_MouseMotionEvent: ";
        std::cout << "type = SDL_MOUSEMOTION";
        std::cout << ", state = " << event->state;
        std::cout << ", x = " << event->x << ", y = " << event->y;
        std::cout << ", xrel = " << event->xrel << ", yrel = " << event->yrel;
        std::cout << ", type = ???" << std::endl;
        */
    }

    return 0;
}

int FaceButtonUI::HandleMouseButtonEvent(SDL_MouseButtonEvent *event)
{
    int x1, x2, y1, y2;

    x1 = this->GetRect()->x;
    x2 = this->GetRect()->x + this->GetRect()->w;
    y1 = this->GetRect()->y;
    y2 = this->GetRect()->y + this->GetRect()->h;

    if (x1 <= event->x && event->x < x2 && y1 <= event->y && event->y < y2) {
        /*
        std::cout << "SDL_MouseButtonEvent: ";
        std::cout << "type = " << (event->type == SDL_MOUSEBUTTONDOWN) ? "SDL_MOUSEBUTTONDOWN" : "SDL_MOUSEBUTTONUP";
        std::cout << ", button = " << (int)event->button;
        std::cout << ", state = " << (event->state == SDL_PRESSED) ? "SDL_PRESSED" : "SDL_RELEASED";
        std::cout << ", clicks = " << (int)event->clicks << std::endl;
        */
        if (event->button == SDL_BUTTON_LEFT) {
            if (event->type == SDL_MOUSEBUTTONDOWN) {
                this->SetStatus(FaceButtonUI::STATUS_FACE_PRESSED);
            } else if (event->type == SDL_MOUSEBUTTONUP) {
                this->SetStatus(FaceButtonUI::STATUS_FACE_UNPRESSED);
                this->window->GameReset();
            }
       }
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////
MineGridUI::MineGridUI(MineGameWindowUI *window)
{
    this->window = window;

    this->mine = new SDL_Texture * [9];
    this->mine_covered = NULL;
    this->mine_flagged = NULL;
    this->mine_open_black = NULL;
    this->mine_open_red = NULL;

    this->game_x = 9;
    this->game_y = 9;

    this->rect = new SDL_Rect();
    this->rect->x = 0;
    this->rect->y = 0;
    this->rect->w = this->game_x * MINE_GRID_MINE_SIZE + MINE_GRID_EDGE_MARGIN * 2;
    this->rect->h = this->game_y * MINE_GRID_MINE_SIZE + MINE_GRID_EDGE_MARGIN * 2;

    this->grid_texture = NULL;
}

MineGridUI::~MineGridUI()
{
    this->ReleaseResources();

    delete [] this->mine;
    delete this->rect;
}

void MineGridUI::SetLocation(int x, int y)
{
    this->rect->x = x;
    this->rect->y = y;
}

void MineGridUI::SetGameSize(int x, int y)
{
    std::cout << "MineGridUI::SetGridSize(x=" << x << ", y=" << y << ")" << std::endl;

    if (this->game_x != x || this->game_y != y) {
        this->game_x = x;
        this->game_y = y;

        this->rect->w = this->game_x * MINE_GRID_MINE_SIZE + MINE_GRID_EDGE_MARGIN * 2;
        this->rect->h = this->game_y * MINE_GRID_MINE_SIZE + MINE_GRID_EDGE_MARGIN * 2;

        if (this->grid_texture != NULL) {
            SDL_DestroyTexture(this->grid_texture);
            this->grid_texture = NULL;
        }
    }

    // set game size will always trigger redraw
    this->InitTexture();
}

const SDL_Rect *MineGridUI::GetRect() const
{
    return this->rect;
}

int MineGridUI::GetWidth() const
{
    return this->GetRect()->w;
}

int MineGridUI::GetHeight() const
{
    return this->GetRect()->h;
}

int MineGridUI::LoadResources()
{
    this->mine[0] = this->window->LoadTextureFromFile("./images/png/type0.png");
    this->mine[1] = this->window->LoadTextureFromFile("./images/png/type1.png");
    this->mine[2] = this->window->LoadTextureFromFile("./images/png/type2.png");
    this->mine[3] = this->window->LoadTextureFromFile("./images/png/type3.png");
    this->mine[4] = this->window->LoadTextureFromFile("./images/png/type4.png");
    this->mine[5] = this->window->LoadTextureFromFile("./images/png/type5.png");
    this->mine[6] = this->window->LoadTextureFromFile("./images/png/type6.png");
    this->mine[7] = this->window->LoadTextureFromFile("./images/png/type7.png");
    this->mine[8] = this->window->LoadTextureFromFile("./images/png/type8.png");
    this->mine_covered = this->window->LoadTextureFromFile("./images/png/closed.png");
    this->mine_flagged = this->window->LoadTextureFromFile("./images/png/flag.png");
    this->mine_flagged_wrong = this->window->LoadTextureFromFile("./images/png/mine_wrong.png");
    this->mine_open_black = this->window->LoadTextureFromFile("./images/png/mine.png");
    this->mine_open_red = this->window->LoadTextureFromFile("./images/png/mine_red.png");

    this->InitTexture();

    return 0;
}

void MineGridUI::ReleaseResources()
{
    for (int i = 0; i < 9; i++) {
        if (this->mine[i] != NULL) {
            SDL_DestroyTexture(this->mine[i]);
            this->mine[i] = NULL;
        }
    }

    if (this->mine_covered != NULL) {
        SDL_DestroyTexture(this->mine_covered);
        this->mine_covered = NULL;
    }

    if (this->mine_flagged != NULL) {
        SDL_DestroyTexture(this->mine_flagged);
        this->mine_flagged = NULL;
    }

    if (this->mine_flagged_wrong != NULL) {
        SDL_DestroyTexture(this->mine_flagged_wrong);
        this->mine_flagged_wrong = NULL;
    }

    if (this->mine_open_black != NULL) {
        SDL_DestroyTexture(this->mine_open_black);
        this->mine_open_black = NULL;
    }

    if (this->mine_open_red != NULL) {
        SDL_DestroyTexture(this->mine_open_red);
        this->mine_open_red = NULL;
    }

    if (this->grid_texture != NULL) {
        SDL_DestroyTexture(this->grid_texture);
        this->grid_texture = NULL;
    }
}

int MineGridUI::Redraw()
{
    this->window->UpdateWindowTexture(this->grid_texture, this->GetRect());

    return 0;
}

int MineGridUI::HandleMouseMotionEvent(SDL_MouseMotionEvent *event)
{
    int x1, x2, y1, y2, last_x, last_y;
    bool start_in, end_in;

    x1 = this->GetRect()->x;
    x2 = this->GetRect()->x + this->GetRect()->w;
    y1 = this->GetRect()->y;
    y2 = this->GetRect()->y + this->GetRect()->h;
    last_x = event->x - event->xrel;
    last_y = event->y - event->yrel;

    start_in = (x1 <= last_x && last_x < x2 && y1 <= last_y && last_y < y2);
    end_in = (x1 <= event->x && event->x < x2 && y1 <= event->y && event->y < y2);

    // leaving component
    if (start_in && !end_in) {
        /*
        std::cout << "SDL_MouseMotionEvent: ";
        std::cout << "type = SDL_MOUSEMOTION";
        std::cout << ", state = " << event->state;
        std::cout << ", x = " << event->x << ", y = " << event->y;
        std::cout << ", xrel = " << event->xrel << ", yrel = " << event->yrel;
        std::cout << ", type = leaving" << std::endl;
        */
    }
    // entering component
    else if (!start_in && end_in) {
        /*
        std::cout << "SDL_MouseMotionEvent: ";
        std::cout << "type = SDL_MOUSEMOTION";
        std::cout << ", state = " << event->state;
        std::cout << ", x = " << event->x << ", y = " << event->y;
        std::cout << ", xrel = " << event->xrel << ", yrel = " << event->yrel;
        std::cout << ", type = entering" << std::endl;
        */
    } else {
        /*
        std::cout << "SDL_MouseMotionEvent: ";
        std::cout << "type = SDL_MOUSEMOTION";
        std::cout << ", state = " << event->state;
        std::cout << ", x = " << event->x << ", y = " << event->y;
        std::cout << ", xrel = " << event->xrel << ", yrel = " << event->yrel;
        std::cout << ", type = ???" << std::endl;
        */
    }

    return 0;
}

int MineGridUI::HandleMouseButtonEvent(SDL_MouseButtonEvent *event)
{
    int x1, x2, y1, y2;

    x1 = this->GetRect()->x;
    x2 = this->GetRect()->x + this->GetRect()->w;
    y1 = this->GetRect()->y;
    y2 = this->GetRect()->y + this->GetRect()->h;

    if (x1 <= event->x && event->x < x2 && y1 <= event->y && event->y < y2) {
/*
        std::cout << "SDL_MouseButtonEvent: ";
        std::cout << "type = " << ((event->type == SDL_MOUSEBUTTONDOWN) ? "SDL_MOUSEBUTTONDOWN" : "SDL_MOUSEBUTTONUP");
        std::cout << ", button = " << (int)event->button;
        std::cout << ", state = " << (event->state == SDL_PRESSED) ? "SDL_PRESSED" : "SDL_RELEASED";
        std::cout << ", clicks = " << (int)event->clicks << std::endl;
*/
        if (event->x - x1 > MINE_GRID_EDGE_MARGIN && event->y - y1 > MINE_GRID_EDGE_MARGIN) {
            int index_x = (event->x - x1) / MINE_GRID_MINE_SIZE;
            int index_y = (event->y - y1) / MINE_GRID_MINE_SIZE;

            if (event->button == SDL_BUTTON_LEFT && event->type == SDL_MOUSEBUTTONUP) {
                this->window->GameOpen(index_x, index_y);
            } else if (event->button == SDL_BUTTON_RIGHT && event->type == SDL_MOUSEBUTTONUP) {
                this->window->GameTouchFlag(index_x, index_y);
            }   

            this->RedrawDirtyGrids();
        }
    }
  
    return 0;
}

int MineGridUI::InitTexture()
{
    if (this->grid_texture == NULL) {
        SDL_Texture *t;

        // get grid_texture 
        t = this->window->CreateTexture(this->GetWidth(), this->GetHeight());
        if (t == NULL) {
            return -1;
        }

        this->grid_texture = t;
    }

    // draw grid_texture with mines
    for (int i = 0; i < this->game_y; i++) {
        for (int j = 0; j < this->game_x; j++) {
            SDL_Rect rect;

            rect.x = j * MINE_GRID_MINE_SIZE + 2;
            rect.y = i * MINE_GRID_MINE_SIZE + 2;
            rect.w = MINE_GRID_MINE_SIZE;
            rect.h = MINE_GRID_MINE_SIZE;

            this->window->UpdateTexture(this->grid_texture, this->mine_covered, &rect);
        }
    }

    return 0;
}

int MineGridUI::RedrawDirtyGrids()
{
    std::vector<MineGameGrid> grids;

    this->window->GameGetDirtyGrids(grids);

    for (size_t i = 0; i < grids.size(); i++) {
        SDL_Rect rect;

        rect.x = grids[i].x * MINE_GRID_MINE_SIZE + MINE_GRID_EDGE_MARGIN;
        rect.y = grids[i].y * MINE_GRID_MINE_SIZE + MINE_GRID_EDGE_MARGIN;
        rect.w = MINE_GRID_MINE_SIZE;
        rect.h = MINE_GRID_MINE_SIZE;

        std::cout << "MineGridUI: redraw grid with state = " << grids[i].state <<" at (" << grids[i].x << ", " << grids[i].y << ")" << std::endl;

        switch (grids[i].state) {
        case MineGameGrid::State::STATE_COVERED:
            this->window->UpdateTexture(this->grid_texture, this->mine_covered, &rect);
            break;

        case MineGameGrid::State::STATE_FLAGGED:
            this->window->UpdateTexture(this->grid_texture, this->mine_flagged, &rect);
            break;

        case MineGameGrid::State::STATE_FLAGGED_WRONG:
            this->window->UpdateTexture(this->grid_texture, this->mine_flagged_wrong, &rect);
            break;

        case MineGameGrid::State::STATE_MINE_EXPLODE:
            this->window->UpdateTexture(this->grid_texture, this->mine_open_red, &rect);
            break;

        case MineGameGrid::State::STATE_MINE_OPEN:
            this->window->UpdateTexture(this->grid_texture, this->mine_open_black, &rect);
            break;

        case MineGameGrid::State::STATE_MINE_0:
            this->window->UpdateTexture(this->grid_texture, this->mine[0], &rect);
            break;

        case MineGameGrid::State::STATE_MINE_1:
            this->window->UpdateTexture(this->grid_texture, this->mine[1], &rect);
            break;

        case MineGameGrid::State::STATE_MINE_2:
            this->window->UpdateTexture(this->grid_texture, this->mine[2], &rect);
            break;

        case MineGameGrid::State::STATE_MINE_3:
            this->window->UpdateTexture(this->grid_texture, this->mine[3], &rect);
            break;

        case MineGameGrid::State::STATE_MINE_4:
            this->window->UpdateTexture(this->grid_texture, this->mine[4], &rect);
            break;

        case MineGameGrid::State::STATE_MINE_5:
            this->window->UpdateTexture(this->grid_texture, this->mine[5], &rect);
            break;

        case MineGameGrid::State::STATE_MINE_6:
            this->window->UpdateTexture(this->grid_texture, this->mine[6], &rect);
            break;

        case MineGameGrid::State::STATE_MINE_7:
            this->window->UpdateTexture(this->grid_texture, this->mine[7], &rect);
            break;

        case MineGameGrid::State::STATE_MINE_8:
            this->window->UpdateTexture(this->grid_texture, this->mine[8], &rect);
            break;

        default:
            std::cerr << "MineGridUI::RedrawDirtyGrids: unknown grid state " << grids[i].state << std::endl;
            break;
        }
    }

    this->Redraw();

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////
