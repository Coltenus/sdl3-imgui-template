#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "button.h"
#include <vector>

class Titlebar {
private:
    SDL_FPoint size_;
    SDL_Color color;
    SDL_Texture* texture;
    SDL_FRect text_rect;
    SDL_Renderer* ren;
    std::vector<Button*> buttons;
    int event_type_, dragging;
    SDL_FPoint offset;
    
public:
    Titlebar(SDL_Renderer* ren, SDL_Window* win, const char* title, float height, SDL_Color color, TTF_Font* font);
    ~Titlebar();
    void draw();
    int events(SDL_Event* e);

    SDL_FPoint size() { return size_; }
    int event_type() { return event_type_; }
};

#endif
