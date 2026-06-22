#include <SDL.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>

float angleX = 0;
float angleY = 0;

typedef struct p3 {
    float x, y, z;
} p3;

typedef struct Triangle {
    int v0, v1, v2;
} Triangle;

#define GRID_SIZE 100
#define NUMPOINTS (GRID_SIZE * GRID_SIZE)
#define NUMTRIANGLES ((GRID_SIZE-1) * (GRID_SIZE-1) *2)

p3 Model[NUMPOINTS];
p3 Rotated[NUMPOINTS];
Triangle Triangles[NUMTRIANGLES];
// global Z-Buffer
float *zBuffer = NULL;

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

void create_model(void){
    int i = 0;
    // Generate Points
    for (int x = 0; x < GRID_SIZE; x++) {
        for (int z = 0; z < GRID_SIZE; z++) {
            float fx = (float)x - (GRID_SIZE / 2.0f);
            float fz = (float)z - (GRID_SIZE / 2.0f);
            
            float y = 10 + (sin(fx/10.0f * 3.14f) * 5) + (cos(fz/10.0f * 3.14f) * 5);
            
            Model[i].x = fx * 4;
            Model[i].y = y * 4;
            Model[i].z = fz * 4;
            i++;
        }
    }
    // Linking the points
    int t = 0;
    for (int x = 0; x < GRID_SIZE-1; x++) {
        for (int z = 0; z < GRID_SIZE-1; z++) {
            int topLeft = x * GRID_SIZE +z;
            int topRight = topLeft+1;
            int bottomLeft = (x+1) * GRID_SIZE +z;
            int bottomRight = bottomLeft +1;
            
            // First triangle
            Triangles[t].v0 = topLeft;
            Triangles[t].v1 = bottomLeft;
            Triangles[t].v2 = topRight;
            t++;

            // Second triangle
            Triangles[t].v0 = topRight;
            Triangles[t].v1 = bottomLeft;
            Triangles[t].v2 = bottomRight;
            t++;
        }
    }
}
void create_model2(void) {
#if 0
    for (int j = 0; j < NUMPOINTS; j++) {
        Model[j].x = -150 + rand() % 300;
        Model[j].y = -150 + rand() % 300;
        Model[j].z = -150 + rand() % 300;
    }e
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

float edgeFunction(float x0, float y0, float x1, float y1, float px, float py) {
    return (px - x0) * (y1 - y0) - (py - y0) * (x1 - x0);
}

// Raster Function
void draw_triangle(SDL_Surface *surface, float x0, float y0, float z0,
                                         float x1, float y1, float z1,
                                         float x2, float y2, float z2, 
                                         int r, int g, int b)
{
    // Search Bounding Box
    int minX = (int)fmax(0, fmin(x0, fmin(x1, x2)));
    int minY = (int)fmax(0, fmin(y0, fmin(y1, y2)));
    int maxX = (int)fmin(surface->w - 1, fmax(x0, fmax(x1, x2)));
    int maxY = (int)fmin(surface->h - 1, fmax(y0, fmax(y1, y2)));

    float area = edgeFunction(x0, y0, x1, y1, x2, y2);
    if (fabs(area) < 0.001f) return; // invisible triangle

    float sign = (area > 0) ? 1.0f : -1.0f;

    // Rastering points into triangle 
    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            float px = x + 0.5f; // centre of pixel
            float py = y + 0.5f;

            // Calcola i "pesi" Baricentrici
            float w0 = edgeFunction(x1, y1, x2, y2, px, py) * sign;
            float w1 = edgeFunction(x2, y2, x0, y0, px, py) * sign;
            float w2 = edgeFunction(x0, y0, x1, y1, px, py) * sign;

            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                // Normalized weights
                w0 /= fabs(area);
                w1 /= fabs(area);
                w2 /= fabs(area);

                // Depth of Z
                float pz = w0 * z0 + w1 * z1 + w2 * z2;
                
                int index = y * surface->w + x;
                // Test Z-Buffer
                if (pz < zBuffer[index]) {
                    zBuffer[index] = pz; // Save the new depth
                    pixel(surface, x, y, r, g, b);
                }
            }
        }
    }
}

void draw(SDL_Surface *surface) {
    int width = surface->w;
    int height = surface->h;

    float cx = (float)width / 2;
    float cy = (float)height / 2;

    //fade(surface);
    clear(surface);
    for(int i = 0; i < width *height; i++){
        zBuffer[i] = 10000.0f;
    }

    rotate();

    // Array to save a 2D coordinate
    p3 Projected[NUMPOINTS];
    for (int j = 0; j < NUMPOINTS; j++) {
        float zfactor = 1 + (Rotated[j].z / 300);
        if(zfactor < 0.01f) zfactor = 0.1f;
        
        Projected[j].x = cx + (Rotated[j].x / zfactor);
        Projected[j].y = cy + (Rotated[j].y / zfactor);
        Projected[j].z = Rotated[j].z;
    }

    // Drawing triangles
    for (int i = 0; i < NUMTRIANGLES; i++) {
        int i0 = Triangles[i].v0;
        int i1 = Triangles[i].v1;
        int i2 = Triangles[i].v2;

        p3 p0 = Projected[i0];
        p3 p1 = Projected[i1];
        p3 p2 = Projected[i2];

        // Color based on first (z value)
        float current_y = Model[i0].y;
        float norm = current_y / 80.0f;
        // Normalize norm to be between 0 and 1 and calculate gradient
        if(norm < 0.0f) norm = 0.0f;
        if(norm > 1.0f) norm = 1.0f;
        norm = 1.0f - norm;

        int r = 30 + (int)(norm * (255-30));
        int g = 160 + (int)(norm * (255-160));
        int b = 30 + (int)(norm * (255-30));
        draw_triangle(surface, p0.x, p0.y, p0.z,
                               p1.x, p1.y, p1.z,
                               p2.x, p2.y, p2.z, r, g, b);
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

    zBuffer = (float*)malloc(640 * 480 * sizeof(float));
    
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

