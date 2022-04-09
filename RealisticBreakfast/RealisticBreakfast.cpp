#include <iostream>
#include <time.h>
#include <stdio.h>
#include <string>
#include <ctime>
#include "SDL.h"
#include "float.h"
#include "vector3D.h"
#include "point3D.h"
#include "color.h"
#include "ray.h"
#include "geometry.h"
#include "sphere.h"
#include "disk.h"
#include "rectangle.h"
#include "triangle.h"
#include "instance.h"
#include "raster.h"
#include "object.h"
#include "camera.h"
#include "color.h"
#include "scene.h"
#include "material.h"
#include "mesh.h"

int const nx = 1280;
int const ny = 720;
int const ns = 800;	// number of samples
int const nr = 8; // number ray per path

scene world;

using namespace std;


int fail = 0;

int init() {
    /* // Initialize SDL2. */
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    /* Create the window where we will draw. */
    window = SDL_CreateWindow("Realistic Breakfast", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, nx, ny, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    //Initialize PNG,JPG,TIF loading
    int flags = IMG_INIT_JPG;
    int initted = IMG_Init(flags);
    if ((initted & flags) != flags) {
        cout << "SDL_image could not initialize! SDL_image Error:" << IMG_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    /* We must call SDL_CreateRenderer in order for draw calls to affect this window. */
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        SDL_DestroyWindow(window);
        cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    return 0;
}

void close() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}

void add_component(const char* path, material* material, vector3D scale_vector, vector3D translation_vector) {
    object* model = new mesh(path);
    instance* mesh = new instance(model, material);
    mesh-> scale(scale_vector.x, scale_vector.y, scale_vector.z);
    mesh-> translate(translation_vector.x, translation_vector.y, translation_vector.z);
    world.addObject(mesh);
}

instance* get_obj(const char* texture) {
    material* material = new lambertian(new image_texture(texture));
    object* sphere_model = new sphere();
    instance* sphere_ptr = new instance(sphere_model, material);  
    return sphere_ptr;
}

color transform_RGB(int r, int g, int b) {
    return color(float(r / 255.0f), float(g / 255.0f), float(b / 255.0f));
}

void cornel_box() {
    material* wall = new lambertian(new image_texture("../models/texture/tiles.jpeg"));
    material* floor = new metal(color(0.6f, 0.6f, 0.6f), 0.2f);
    material* ceil = floor;
    material* light = new diffuse_light(new constant_texture(color(2, 2, 2)));

    // Window 1    
    object* rectangle_model = new rectangle(point3D(-600.0, 50.0, 0), point3D(-450.0, 50.0, 0), point3D(-450, 500.8, 0), point3D(-600.0, 500.8, 0));
    instance* rectangle_ptr = new instance(rectangle_model, light);
    world.addObject(rectangle_ptr);

    // Window 2
    rectangle_model = new rectangle(point3D(-400, 50.0, 0), point3D(-250.0, 50.0, 0), point3D(-250, 500.8, 0), point3D(-400.0, 500.8, 0));
    rectangle_ptr = new instance(rectangle_model, light);
    world.addObject(rectangle_ptr);

    // Window 3
    rectangle_model = new rectangle(point3D(250, 50.0, 0), point3D(400.0, 50.0, 0), point3D(400, 500.8, 0), point3D(250.0, 500.8, 0));
    rectangle_ptr = new instance(rectangle_model, light);
    world.addObject(rectangle_ptr);

    // Window 4
    rectangle_model = new rectangle(point3D(450, 50.0, 0), point3D(600.0, 50.0, 0), point3D(600, 500.8, 0), point3D(450.0, 500.8, 0));
    rectangle_ptr = new instance(rectangle_model, light);
    world.addObject(rectangle_ptr);

    // Frontal light
    disk* spherical_light = new disk(point3D(0.0, 200.8, 1500.0), vector3D(0, 0, -1), 900);
    instance* disk_ptr = new instance(spherical_light, light);
    world.addObject(disk_ptr);

    // Back wall
    rectangle_model = new rectangle(point3D(-650, 0.0, 0), point3D(650.0, 0.0, 0), point3D(650, 548.8, 0), point3D(-650.0, 548.8, 0));
    rectangle_ptr = new instance(rectangle_model, wall);
    world.addObject(rectangle_ptr);

    // Right wall
    rectangle_model = new rectangle(point3D(650, 0.0, 0), point3D(650, 0.0, 2000.0), point3D(650, 548.8, 2000.0), point3D(650, 548.8, 0));
    rectangle_ptr = new instance(rectangle_model, wall);
    world.addObject(rectangle_ptr);

    // Left wall
    rectangle_model = new rectangle(point3D(-650, 0.0, 2000.0), point3D(-650, 0.0, 0), point3D(-650.0, 548.8, 0), point3D(-650.0, 548.8, 2000.0));
    rectangle_ptr = new instance(rectangle_model, wall);
    world.addObject(rectangle_ptr);

    // Ceiling
    rectangle_model = new rectangle(point3D(-650.0, 548.8, 2000.0), point3D(-650.0, 548.8, 0), point3D(650, 548.8, 0), point3D(650, 548.8, 2000.0));
    rectangle_ptr = new instance(rectangle_model, ceil);
    world.addObject(rectangle_ptr);

    // Floor
    rectangle_model = new rectangle(point3D(-650, 0.0, 2000.0), point3D(650, 0.0, 2000.0), point3D(650, 0.0, 0), point3D(-650, 0.0, 0));
    rectangle_ptr = new instance(rectangle_model, floor);
    world.addObject(rectangle_ptr);
    
    point3D lookfrom(0, 90, 1500);
    point3D lookat(0, 200, -1);
    vector3D up(0, 1, 0);
    world.setCamera(lookfrom, lookat, up, 45, nx, ny, ns, nr);
}

void add_tea_glass(vector3D scale_vector, vector3D translation_vector) {
    // Colors definition
    color neutral(1.0f, 1.0f, 1.0f);
    color tea_color = transform_RGB(165, 55, 1);

    // Definition of refractive indices of materials
    float air = 1.0f;
    float glass = 1.5f;
    float tea = 1.34f;

    // Materials definition
    // Air-glass contact
    material* glass_air = new dielectric(glass, air, neutral, neutral);

    // Air-liquid contact
    material* tea_air = new dielectric(tea, air, tea_color, neutral);

    // Liquid
    material* glass_tea = new dielectric(tea, glass, tea_color, neutral);

    add_component("../models/teaGlass/AirGlassContact.obj", glass_air, scale_vector, translation_vector);
    add_component("../models/teaGlass/AirLiquidContact.obj", tea_air, scale_vector, translation_vector);
    add_component("../models/teaGlass/Liquid.obj", glass_tea, scale_vector, translation_vector);
    add_component("../models/teaGlass/Glass.obj", glass_air, scale_vector, translation_vector);
    add_component("../models/teaGlass/Straw.obj", new lambertian(new constant_texture(color(0, 0, 0))), scale_vector, translation_vector);
}

void add_cup_of_coffee() {
    material* ceramic = new lambertian(new constant_texture(color(1, 1, 1)));
    const char* path = "../models/cupOfCoffee/cupOfCoffee.obj";
    add_component(path, ceramic, vector3D(12, 12, 12), vector3D(140, 0, 1050));
}

void add_juice_glass(vector3D scale_vector, vector3D translation_vector) {
    // Colors definition
    color neutral(1.0f, 1.0f, 1.0f);
    color orange_juice_color(0.95f, 0.62f, 0.22f);

    // Definition of refractive indices of materials
    float air = 1.0f;
    float glass = 1.5f;

    // Materials definition
    // Air-glass contact
    material* glass_air = new dielectric(glass, air, neutral, neutral);

    add_component("../models/juiceGlass/AirGlassContact.obj", glass_air, scale_vector, translation_vector);
    add_component("../models/juiceGlass/AirLiquidContact.obj", new lambertian(new constant_texture(orange_juice_color)), scale_vector, translation_vector);
    add_component("../models/juiceGlass/Liquid.obj", new lambertian(new constant_texture(orange_juice_color)), scale_vector, translation_vector);
    add_component("../models/juiceGlass/WaterGlass.obj", glass_air, scale_vector, translation_vector);
}


void add_jug(vector3D scale_vector, vector3D translation_vector) {
    // Colors definition
    color neutral(1.0f, 1.0f, 1.0f);
    color tea_color = transform_RGB(165, 55, 1);
    
    // Definition of refractive indices of materials
    float air = 1.0f;
    float glass = 1.5f;
    float tea = 1.34f;

    // Materials definition
    // Air-glass contact
    material* glass_air = new dielectric(glass, air, neutral, neutral);

    // Air-liquid contact
    material* tea_air = new dielectric(tea, air, tea_color, neutral);

    // Liquid
    material* glass_tea = new dielectric(tea, glass, tea_color, neutral);

    add_component("../models/jug/AirGlassContact.obj", glass_air, scale_vector, translation_vector);
    add_component("../models/jug/AirLiquidContact.obj", tea_air, scale_vector, translation_vector);
    add_component("../models/jug/Liquid.obj", glass_tea, scale_vector, translation_vector);
    add_component("../models/jug/Jug.obj", glass_air, scale_vector, translation_vector);
}


void add_oranges() {
    //Oranges
    const char* texture = "../models/texture/orange.jpeg";
    instance* orange = get_obj(texture);
    orange->scale(30.0, 30.0, 30.0);
    orange->translate(-240, 30, 1100);
    world.addObject(orange);

    orange = get_obj(texture);
    orange->scale(30.0, 30.0, 30.0);
    orange->translate(-95, 30, 1050);
    world.addObject(orange);

    orange = get_obj(texture);
    orange->scale(50.0, 50.0, 50.0);
    orange->translate(-220, 50, 880);
    world.addObject(orange);
}

void add_cookies() {
    //Cookies
    const char* texture = "../models/texture/cookie.jpeg";
    instance* cookie = get_obj(texture);
    cookie->scale(30.0, 7, 30.0);
    cookie->translate(100, 0, 1150);
    world.addObject(cookie);

    cookie = get_obj(texture);
    cookie->scale(30.0, 7, 30.0);
    cookie->translate(115, 7, 1150);
    world.addObject(cookie);

    cookie = get_obj(texture);
    cookie->scale(30.0, 7, 30.0);
    cookie->translate(135, 0, 1150);
    world.addObject(cookie);
}

void add_flowerpot(vector3D scale_vector, vector3D translation_vector) {
    // Colors definition
    color neutral(1.0f, 1.0f, 1.0f);
    
    // Definition of refractive indices of materials
    float air = 1.0f;
    float glass = 1.5f;
    float water = 1.33f;

    // Materials definition
    // Air-glass contact
    material* glass_air = new dielectric(glass, air, neutral, neutral);

    // Air-liquid contact
    material* water_air = new dielectric(water, air, neutral, neutral);

    // Liquid
    material* glass_water = new dielectric(water, glass, neutral, neutral);

    add_component("../models/flowerpot/AirGlassContact.obj", glass_air, scale_vector, translation_vector);
    add_component("../models/flowerpot/AirLiquidContact.obj", water_air, scale_vector, translation_vector);
    add_component("../models/flowerpot/Liquid.obj", glass_water, scale_vector, translation_vector);
    add_component("../models/flowerpot/Flowerpot.obj", glass_air, scale_vector, translation_vector);
    add_component("../models/flowerpot/FlowerStem.obj", new lambertian(new constant_texture(color(0.16f, 0.45f, 0.2f))), scale_vector, translation_vector);
    add_component("../models/flowerpot/Flower.obj", new lambertian(new constant_texture(color(1, 1, 0))), scale_vector, translation_vector);
}

void create_final_scene() {
    add_tea_glass(vector3D(0.3, 0.3, 0.3), vector3D(-90, 0, 1150));
    add_juice_glass(vector3D(45, 45, 45), vector3D(250, 0, 950));
    add_flowerpot(vector3D(130, 130, 130), vector3D(0, 0, 700));
    add_jug(vector3D(120, 120, 120), vector3D(-250, 0, 950));
    add_cup_of_coffee();
    add_oranges();
    add_cookies();
}

void create_juice_scene() {
    // Obj translation and obj scaling
    vector3D scale_vector = vector3D(50, 50, 50);
    vector3D translation_vector = vector3D(0, 0, 1150);
    add_juice_glass(scale_vector, translation_vector);

}

void create_tea_scene() {
    // Obj translation and obj scaling
    vector3D scale_vector = vector3D(0.3, 0.3, 0.3);
    vector3D translation_vector = vector3D(55, 0, 1150);

    add_tea_glass(scale_vector, translation_vector);
}

void create_jug_scene() {
    // Obj translation and obj scaling
    vector3D scale_vector = vector3D(85, 85, 85);
    vector3D translation_vector = vector3D(-65, 0, 1100);

    add_jug(scale_vector, translation_vector);
    create_tea_scene();
}



void create_flowerpot_scene() {
    // Obj translation and obj scaling
    vector3D scale_vector = vector3D(60, 60, 60);
    vector3D translation_vector = vector3D(0, 0, 1220);
    add_flowerpot(scale_vector, translation_vector);
}

int main(int argc, char* argv[])
{
    if (init() == 1) {
        cout << "App Error! " << std::endl;
        return 1;
    }

    time_t start, end;
    time(&start);
    cornel_box();
    create_final_scene();
    //create_flowerpot_scene();
    //create_tea_scene();
    //create_jug_scene();
    world.parallel_render();

    time(&end);
    double dif = difftime(end, start);
    printf("Rendering time: %.2lf seconds.", dif);
    printf("False points: %d", fail);

    SDL_Event event;
    bool quit = false;

    /* Poll for events */
    while (SDL_PollEvent(&event) || (!quit)) {

        switch (event.type) {

        case SDL_QUIT:
            quit = true;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
                quit = true;
                break;
                // cases for other keypresses

            case SDLK_s:
                saveScreenshotBMP("../render/screenshot_" + to_string(start) + ".bmp");
                break;
            }
        }
    }
    close();
    return 0;
}
