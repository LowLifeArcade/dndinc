#include <stdio.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_rect.h>
#include <stdbool.h>
#include <time.h>

#ifndef DEBUG
#define DEBUG 0
#endif

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 400

#define PLAYER_HEIGHT 40
#define PLAYER_WIDTH 20

typedef struct EntityCfg {
    int height, width, health;
} EntityCfg;

typedef struct Movement {
    bool up, down, left, right;
} Movement;

typedef enum GameState {
    GAME_MENU,
    GAME_RUNNING,
    GAME_END,
} GameState;

typedef struct Game {
    GameState state;
} Game;

// game
int isRunning = 1;
double delta;
time_t start, end;
Movement movement;
Game game = {GAME_MENU};
SDL_Renderer *renderer;

// entities
SDL_FRect player;
SDL_FRect enemy;

#define MENU_HEIGHT 350
#define MENU_WIDTH 500
SDL_FRect menu = {(WINDOW_WIDTH / 2) - (MENU_WIDTH / 2), (WINDOW_HEIGHT / 2) - (MENU_HEIGHT / 2), MENU_WIDTH, MENU_HEIGHT};

SDL_FRect createRect(EntityCfg e) {
    return (SDL_FRect) {
        .x = (WINDOW_WIDTH / 2) - (e.width / 2),
        .y = (WINDOW_HEIGHT / 2) - (e.height / 2),
        .h = e.height,
        .w = e.width
    };
}

void startGame() {
    // spawn
    player = createRect((EntityCfg){PLAYER_HEIGHT, PLAYER_WIDTH, 100});
    enemy = createRect((EntityCfg){30, 20, 100});
    enemy.x = enemy.x + 30;
}

void handleInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (DEBUG) {
            char buf[256];
            SDL_GetEventDescription(&event, buf, sizeof buf);
            printf("event = %s\n", buf);
        }

        if (event.type == SDL_EVENT_QUIT || (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE)) {
            puts("quitting\n");
            isRunning = 0;
            break;
        }

        if (game.state == GAME_MENU) {
            if (event.key.key == SDLK_S) {
                game.state = GAME_RUNNING;
                startGame();
            }

            return;
        }

        if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) {
            switch (event.key.key) {
                case SDLK_D:
                    movement.right = event.type == SDL_EVENT_KEY_DOWN;
                    break;

                case SDLK_A:
                    movement.left = event.type == SDL_EVENT_KEY_DOWN;
                    break;

                case SDLK_W:
                    movement.up = event.type == SDL_EVENT_KEY_DOWN;
                    break;

                case SDLK_S:
                    movement.down = event.type == SDL_EVENT_KEY_DOWN;
                    break;

                default:
                    break;
            }
        }
    }
}

void update() {
    delta = difftime(end, start);
    start = time(NULL);

    if (movement.up) {
        player.y--;
    }

    if (movement.down) {
        player.y++;
    }

    if (movement.left) {
        player.x--;
    }

    if (movement.right) {
        player.x++;
    }

    end = time(NULL);
}

void render() {
    if (game.state == GAME_MENU) {
        // clear previous frame
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 200, 200, 0, 255);
        SDL_RenderRect(renderer, &menu);

    } else if (game.state == GAME_RUNNING) {
        // clear previous frame
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // draw new frame
        SDL_SetRenderDrawColor(renderer, 100, 100, 0, 255);
        SDL_RenderRect(renderer, &player);
    }

    // render new frame
    SDL_RenderPresent(renderer);
}

int main(void) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow(
        "Fun Times",
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_MOUSE_FOCUS
    );

    if (!window) {
        printf("no window");
        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(window, NULL);

    if (!renderer) {
        printf("no renderer");
        SDL_Quit();
        return 1;
    }

    while (isRunning) {
        handleInput();
        update();
        render();
    }


    SDL_Delay(1000);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}