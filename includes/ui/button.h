#ifndef BUTTON_H
#define BUTTON_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

namespace ui {

class Button {
private:
    SDL_FRect rect_, text_rect;
    SDL_Color color;
    SDL_Texture* texture;
    SDL_Renderer* ren;

public:
    Button(SDL_Renderer* ren, SDL_FPoint pos, float height, SDL_Color color, const char* text, TTF_Font* font);
    Button(SDL_Renderer* ren, SDL_FRect rect, SDL_Texture* texture, SDL_Color color);
    ~Button();
    void draw(int* selected, int value);
    int events(SDL_Event* e);
    void set_width(int width);
    void set_gap(int gap);

    SDL_FRect rect() { return rect_; }
};

};

#endif
