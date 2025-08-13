#pragma once

#include "SDL2/SDL_image.h"
#include "SDL2/SDL_ttf.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

using std::string;
using vec_string = std::vector<string>;

#define FILECHOOSER_TITLE "Choose a file"
const int filesPerPage = 10; // Assuming 10 files fit on one page, adjust if needed

class Selector
{
  private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_GameController* controller;

    string title;
    SDL_Texture* backgroundTexture;

    Mix_Chunk* clickSound;
    string font_path;
    TTF_Font* font;

    int chosenFileI{};
    vec_string fileList;
    vec_string filters; // New member to store filters
    bool isCustom;

    void renderCounter(SDL_Renderer* renderer, int currentOption,
        int totalOptions);                              // Declaration of renderCounter
    Mix_Chunk* loadClickSound(const vec_string& paths); // Sound loading method
    void getFileList(string directory, bool recursive);
    void drawFileList();
    void drawTitle(const string& title);
    void drawSelector();
    void drawBackground();

  public:
    Selector(string title, string backgroundImage = "");
    ~Selector();
    void setFolder(string directory, bool recursive = false, vec_string filters = {});
    void setCustom(vec_string customChoices);
    int run();
    string get();
};
