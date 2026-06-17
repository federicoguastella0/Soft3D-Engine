#include <SDL.h>
#include <stdint.h>
#include <math.h>

typedef struct p3 {
    float x, y, z;
} p3;

#define NUMPOINTS 10000
p3 Model[NUMPOINTS];
p3 Rotated[NUMPOINTS];

void clear(SDL_Surface *surface) {
    int height = surface->h;
    int pitch = surface->pitch;
    uint8_t *fb = (uint8_t*)surface->pixels;
    memset(fb, 0, pitch * height);
}

void fade(SDL_Surface *surface) {
    int height = surface->h;
    int pitch = surface->pitch;
    uint8_t *fb = (uint8_t*)surface->pixels;
    int numbytes = pitch * height;
    for (int j = 0; j < numbytes; j++) {
        if (fb[j] > 15) {
            fb[j] -= 15;
        } else {
            fb[j] = 0;
        }
    }
}

void create_model(void) {
#if 1
    for (int j = 0; j < NUMPOINTS; j++) {
        Model[j].x = -150 + rand() % 300;
        Model[j].y = -150 + rand() % 300;
        Model[j].z = -150 + rand() % 300;
    }
#else
    int j = 0;
    for (float x = -50; x < 50; x++) {
        for (float z = -50; z <= 50; z++) {
            float y = 10+(sin(x/100*3.14*5) * 5)+(cos(z/100*3.14*5) * 5);
            Model[j].x = x * 4;
            Model[j].y = y * 4;
            Model[j].z = z * 4;
            j++;
            if (j == NUMPOINTS) return;
        }
    }
#endif
}

void pixel(SDL_Surface *surface, int x, int y, int r, int g, int b) {
    // Write pixels – fill with a horizontal RGB gradient
    int width = surface->w;
    int height = surface->h;
    if (x < 0 || x >= width) return;
    if (y < 0 || y >= height) return;

    int pitch = surface->pitch;               // bytes per row
    uint8_t *fb = (uint8_t*)surface->pixels;

    fb[y * pitch + x * 4 + 0] = b;
    fb[y * pitch + x * 4 + 1] = g;
    fb[y * pitch + x * 4 + 2] = r;
    fb[y * pitch + x * 4 + 3] = 255;
}

void rotate(float time) {
    float alpha = (time * ((3.14*2)/60)) * 0.05;
    for (int j = 0; j < NUMPOINTS; j++) {
        /* Rotation along the y axis is:
         *
         * x' =  x cosθ + z sinθ
         * y' =  y
         * z' = -x sinθ + z cosθ
         */
        Rotated[j].x = Model[j].x * cos(alpha) + Model[j].z * sin(alpha);
        Rotated[j].y = Model[j].y;
        Rotated[j].z = -Model[j].x * sin(alpha) +  Model[j].z * cos(alpha);
    }
}

void draw(SDL_Surface *surface, float time) {
    int width = surface->w;
    int height = surface->h;

    float cx = (float)width / 2;
    float cy = (float)height / 2;

    rotate(time);

    fade(surface);
    clear(surface);
    for (int j = 0; j < NUMPOINTS; j++) {
        float zfactor = 1 + (Rotated[j].z / 300);
        float x = Rotated[j].x / zfactor;
        float y = Rotated[j].y / zfactor;
        pixel(surface,round(cx+x),round(cy+y),255,255,255);
    }
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    // Initialize SDL video
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    // Create window
    SDL_Window* window = SDL_CreateWindow(
        "Pixel Framebuffer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Get the window's surface (the framebuffer)
    SDL_Surface* surface = SDL_GetWindowSurface(window);
    if (!surface) {
        SDL_Log("SDL_GetWindowSurface failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    clear(surface);
    create_model();
    int running = 1;
    float time = 0;
    while(running) {
        draw(surface,time);
        SDL_UpdateWindowSurface(window);
        time += 1;

        // Main loop – wait for quit event
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }
        SDL_Delay(16);
    }

    // Cleanup
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

