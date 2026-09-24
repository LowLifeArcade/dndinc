#include <stdio.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_rect.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#ifndef DEBUG
#define DEBUG 0
#endif

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 400

#define PLAYER_HEIGHT 40
#define PLAYER_WIDTH 20

// game
typedef enum GameState {
    GAME_MENU,
    GAME_RUNNING,
    GAME_END,
} GameState;

typedef enum GameExitState {
    GAME_EXIT_SUCCESS,
    GAME_EXIT_FAIL,
} GameExitState;

typedef struct GameGlobal {
    void (*render)(void);
    void (*update)(void);
    void (*handleInput)(void);
    void (*destroy)(void);
    GameState state;
} GameGlobal;

int isRunning = 1;
double delta;
time_t start, end;
SDL_Renderer *renderer;
SDL_Window *window;
GameGlobal game;

// entities
typedef enum Facing {
    FACING_UP,
    FACING_DOWN,
    FACING_LEFT,
    FACING_RIGHT,
} Facing;
typedef struct Movement {
    bool up, down, left, right;
} Movement;

typedef enum PosState {
    POS_IDLE,
    POS_WALK,
    POS_HURT,
} PosState;
typedef struct Position {
    int x;
    int y;
    PosState state;
} Position;

typedef struct Box {
    int h;
    int w;
    Position pos;
} Box;

typedef enum ActionState {
    ACTION_IDLE,
    ACTION_TALK,
    ACTION_INSPECT,
    ACTION_ATTACK,
} ActionState;

typedef struct Action {
    ActionState current;
    ActionState previous;
} Action;


typedef struct Player {
    int h, w, health;
    Box hitBox;
    Box hurtBox;
    Position pos;
    Action action;
    Facing facing;
} Player;

typedef struct Enemy {
    int h, w, health;
    Position pos;
} Enemy;

Movement movement;
Player player;
Enemy enemy;

// main menu
#define MENU_HEIGHT 350
#define MENU_WIDTH 500
SDL_FRect mainManu = {(WINDOW_WIDTH / 2) - (MENU_WIDTH / 2), (WINDOW_HEIGHT / 2) - (MENU_HEIGHT / 2), MENU_WIDTH, MENU_HEIGHT};

// geometry
void drawRoundedRect(SDL_Renderer *renderer, float x, float y, float w, float h, float r) {
}

void drawCircle(SDL_Renderer *renderer, float centerX, float centerY, float r) {
    // circle algo x² + y² = r²
    for (int x = -r; x <= r; x++) {
        for (int y = -r; y <= r; y++) {
            if (x * x + y * y <= r * r) {
                SDL_RenderPoint(renderer, centerX + x, centerY + y);
            }
        }
    }
}

// game
void startGame() {
    // spawn
    player.h = PLAYER_HEIGHT;
    player.w = PLAYER_WIDTH;
    player.pos.x = (WINDOW_WIDTH / 2) - (player.w / 2);
    player.pos.y = (WINDOW_HEIGHT / 2) - (player.h / 2);
    player.pos.state = POS_IDLE;
    player.action.previous = ACTION_IDLE;
    player.action.current = ACTION_IDLE;
    player.facing = FACING_DOWN;

    Box *hurt = &player.hurtBox;
    hurt->h = player.h - 1;
    hurt->w = player.w - 1;
    hurt->pos = player.pos;

    Box *hit = &player.hitBox;
    hit->h= 20;
    hit->w= 50;
    hit->pos = player.pos;

    enemy.h = 30;
    enemy.w = 20;
    enemy.pos.x = enemy.pos.x + 30;
}

void gameUpdate() {
    delta = difftime(end, start);
    start = time(NULL);

    if (player.action.current == ACTION_ATTACK) {
        DEBUG && puts("attacking");
    }

    bool playerMoving = (movement.up || movement.down || movement.left || movement.right);

    if (movement.up) {
        player.facing = FACING_UP;
        player.pos.y--;
        player.hitBox.pos.y--;
        player.hurtBox.pos.y--;
    }

    if (movement.down) {
        player.facing = FACING_DOWN;
        player.pos.y++;
        player.hitBox.pos.y++;
        player.hurtBox.pos.y++;
    }

    if (movement.left) {
        player.facing = FACING_LEFT;
        player.pos.x--;
        player.hitBox.pos.x--;
        player.hurtBox.pos.x--;
    }

    if (movement.right) {
        player.facing = FACING_RIGHT;
        player.pos.x++;
        player.hitBox.pos.x++;
        player.hurtBox.pos.x++;
    }

    if (playerMoving && player.pos.state != POS_WALK) {
        DEBUG && puts("player walking");
        player.pos.state = POS_WALK;
    } else if (!playerMoving && player.pos.state != POS_IDLE) {
        DEBUG && puts("player idle");
        player.pos.state = POS_IDLE;
    }

    end = time(NULL);
}

void gameHandleInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (DEBUG) {
            char buf[256];
            SDL_GetEventDescription(&event, buf, sizeof buf);
            printf("event = %s\n", buf);
        }

        if (event.type == SDL_EVENT_QUIT || (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE)) {
            DEBUG && puts("quitting\n");
            isRunning = 0;
            return;
        }

        if (game.state == GAME_MENU) {
            if (event.key.key == SDLK_S) {
                game.state = GAME_RUNNING;
                startGame();
            }

            return;
        }

        if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) {
            bool isKeyDown = event.type == SDL_EVENT_KEY_DOWN;
            switch (event.key.key) {
                case SDLK_D:
                    movement.right = isKeyDown;
                    break;

                case SDLK_A:
                    movement.left = isKeyDown;
                    break;

                case SDLK_W:
                    movement.up = isKeyDown;
                    break;

                case SDLK_S:
                    movement.down = isKeyDown;
                    break;

                case SDLK_SPACE:
                    if (isKeyDown && player.action.current != ACTION_ATTACK) {
                        player.action.previous = player.action.current;
                        player.action.current = ACTION_ATTACK;
                    } else if (!isKeyDown && player.action.current == ACTION_ATTACK) {
                        player.action.current = player.action.previous;
                    }

                    break;

                default:
                    break;
            }
        }
    }
}

void gameRender() {
    if (game.state == GAME_MENU) {
        // clear previous frame
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 200, 200, 0, 255);
        // drawCircle(renderer, 200, 200, 40);
        SDL_RenderRect(renderer, &mainManu);

    } else if (game.state == GAME_RUNNING) {
        // clear previous frame
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // draw new frame
        SDL_SetRenderDrawColor(renderer, 100, 100, 0, 255);
        SDL_RenderRect(renderer, &(SDL_FRect){player.hurtBox.pos.x, player.hurtBox.pos.y, player.hurtBox.w, player.hurtBox.h});


        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderRect(renderer, &(SDL_FRect){player.hitBox.pos.x, player.hitBox.pos.y, player.hitBox.w, player.hitBox.h});
    }

    // render new frame
    SDL_RenderPresent(renderer);
}

void gameDestroy() {
    // SDL_Delay(1000);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

bool gameInit() {
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow(
        "Fun Times",
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_MOUSE_FOCUS
    );

    if (!window) {
        puts("no window");
        SDL_Quit();
        return false;
    }

    renderer = SDL_CreateRenderer(window, NULL);

    if (!renderer) {
        puts("no renderer");
        SDL_Quit();
        return false;
    }

    game.handleInput = gameHandleInput;
    game.render = gameRender;
    game.update = gameUpdate;
    game.destroy = gameDestroy;
    game.state = GAME_MENU;

    return true;
}

int main(void) {
    // game methods totally unecessary but makes me feel at javascript home
    if (!gameInit()) {
        return GAME_EXIT_FAIL;
    }

    while (isRunning) {
        game.handleInput();
        game.update();
        game.render();
    }

    game.destroy();

    return GAME_EXIT_SUCCESS;
}