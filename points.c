#include <SDL.h>
#include <stdint.h>
#include <math.h>

float angleX = 0;
float angleY = 0;

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
#if 0
    for (int j = 0; j < NUMPOINTS; j++) {
        Model[j].x = -150 + rand() % 300;
        Model[j].y = -150 + rand() % 300;
        Model[j].z = -150 + rand() % 300;
    }
#else
    // Generate a 3D surface
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

void rotate(void) {
    float cosX = cos(angleX), sinX = sin(angleX);
    float cosY = cos(angleY), sinY = sin(angleY);

    for (int j = 0; j < NUMPOINTS; j++) {
        float x = Model[j].x;
        float y = Model[j].y;
        float z = Model[j].z;

        //Rotation X
        float x1 = x;
        float y1 = y * cosX - z*sinX;
        float z1 = y * sinX + z*cosX;

        //Rotation Y
        float x2 = x1 * cosY + z1 * sinY;
        float y2 = y1;
        float z2 = -x1 * sinY + z1 * cosY;

        Rotated[j].x = x2;
        Rotated[j].y = y2;
        Rotated[j].z = z2;
    }
}

void draw(SDL_Surface *surface) {
    int width = surface->w;
    int height = surface->h;

    float cx = (float)width / 2;
    float cy = (float)height / 2;

    rotate();

    fade(surface);
    //clear(surface);
    for (int j = 0; j < NUMPOINTS; j++) {
        float zfactor = 1 + (Rotated[j].z / 300);
        float x = Rotated[j].x / zfactor;
        float y = Rotated[j].y / zfactor;

        // Color based on depth (z value)
        float current_y = Model[j].y;
        float norm = current_y / 80.0f;
        // Normalize norm to be between 0 and 1 and calculate gradient
        if(norm < 0.0f) norm = 0.0f;
        if(norm > 1.0f) norm = 1.0f;

        norm = 1.0f - norm;
        int r = 30 + (int)(norm * (255-30));
        int g = 160 + (int)(norm * (255-160));
        int b = 30 + (int)(norm * (255-30));
        pixel(surface,round(cx+x),round(cy+y),r,g,b);
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
        draw(surface);
        SDL_UpdateWindowSurface(window);
        time += 1;

        // Main loop – wait for quit event
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
            else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        running = false; // Esci premendo ESC
                        break;
                    case SDLK_UP:
                        angleX += 0.05f; // Ruota verso l'alto
                        break;
                    case SDLK_DOWN:
                        angleX -= 0.05f; // Ruota verso il basso
                        break;
                    case SDLK_LEFT:
                        angleY -= 0.05f; // Ruota a sinistra
                        break;
                    case SDLK_RIGHT:
                        angleY += 0.05f; // Ruota a destra
                        break;
                    default:
                        break;
                }
            }
        }
        SDL_Delay(16);
    }

    // Cleanup
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

