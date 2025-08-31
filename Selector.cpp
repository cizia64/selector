#include "Selector.hpp"

#include <cmath>
#include <dirent.h>
#include <iostream>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <functional>
#include <cstring>
#include <sstream>
#include <chrono>

// no std::filesystem for GCC 7.5

void Selector::renderCounter(SDL_Renderer* renderer, int currentOption, int totalOptions)
{
    SDL_Color textColor = {255, 255, 255, 255};
    if (!counterFont)
    {
        std::cerr << "Counter font not initialized" << std::endl;
        return;
    }

    string counterText = std::to_string(currentOption) + "/" + std::to_string(totalOptions);
    SDL_Surface* textSurface = TTF_RenderText_Solid(counterFont, counterText.c_str(), textColor);
    if (!textSurface)
    {
        std::cerr << "Failed to create text surface: " << TTF_GetError() << std::endl;
        return;
    }
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!textTexture)
    {
        std::cerr << "Failed to create texture: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(textSurface);
        return;
    }

    int windowWidth, windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);
    int textWidth = textSurface->w;
    int textHeight = textSurface->h;
    SDL_Rect dstRect = {windowWidth - textWidth - 10, windowHeight - textHeight - 10, textWidth,
        textHeight};
    SDL_RenderCopy(renderer, textTexture, nullptr, &dstRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

// Number of files displayed per page

Mix_Chunk* Selector::loadClickSound(const vec_string& paths)
{
    // Initialize audio only once before attempting to load sounds
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        std::cerr << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << '\n';
        std::exit(2);
    }

    // Try loading the sound from the specified paths
    for (const auto& path : paths)
    {
        Mix_Chunk* sound = Mix_LoadWAV(path.c_str());
        if (sound != nullptr)
            return sound; // Return the successfully loaded sound
                          // std::cerr << "Failed to load click sound from " << path << "! SDL_mixer
                          // Error: " << Mix_GetError() << '\n';
    }

    // If no sound file could be loaded, close the audio
    Mix_CloseAudio(); // Close the audio system if no sound was loaded
    return nullptr;   // Return nullptr if no path succeeded
}

static bool isRegularFile(const std::string& path)
{
    struct stat st;
    if (stat(path.c_str(), &st) != 0) return false;
    return S_ISREG(st.st_mode);
}

static bool isDirectory(const std::string& path)
{
    struct stat st;
    if (stat(path.c_str(), &st) != 0) return false;
    return S_ISDIR(st.st_mode);
}

void Selector::getFileList(string directory, bool recursive)
{
    auto matchFilters = [&](const std::string& filename) {
        string lower = filename;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        if (filters.empty()) return true;
        for (const auto& f : filters)
        {
            string lf = f;
            std::transform(lf.begin(), lf.end(), lf.begin(), ::tolower);
            if (lower.find(lf) != string::npos) return true;
        }
        return false;
    };

    std::function<void(const std::string&)> scan = [&](const std::string& dir) {
        DIR* d = opendir(dir.c_str());
        if (!d) return;
        struct dirent* ent;
        while ((ent = readdir(d)) != nullptr)
        {
            const char* name = ent->d_name;
            if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) continue;
            std::string full = dir;
            if (!full.empty() && full.back() != '/') full += '/';
            full += name;

            if (isDirectory(full))
            {
                if (recursive) scan(full);
            }
            else if (isRegularFile(full))
            {
                if (matchFilters(name)) fileList.push_back(full);
            }
        }
        closedir(d);
    };

    fileList.clear();
    scan(directory);
    std::sort(fileList.begin(), fileList.end());
}
void Selector::drawFileList()
{
    // Match previous look: original code rendered font size ~100 then divided by 5
    const float baseDiv = 5.0f;
    int rawLine = listFont ? TTF_FontLineSkip(listFont) : 100;
    if (rawLine <= 0) rawLine = 100;
    int lineAdvance = static_cast<int>(rawLine / baseDiv);
    if (lineAdvance < 12) lineAdvance = 12; // minimal spacing guard
    int baseY = 500;

    for (int i = 0; i < static_cast<int>(fileList.size()); ++i)
    {
        int y = baseY - chosenFileI * lineAdvance + i * lineAdvance;

        if (y < 1000 && y > 0)
        {
            SDL_Surface* textSurface = TTF_RenderText_Blended(listFont ? listFont : font, fileList[i].c_str(),
                {255, 255, 255, static_cast<uint8_t>(255 - std::abs(500 - y) / 2)});

            SDL_Rect sourceRect{0, 0, textSurface->w, textSurface->h};
            // Scale down to match previous visual size convention
            SDL_Rect targetRect{0, y, static_cast<int>(textSurface->w / baseDiv),
                static_cast<int>(textSurface->h / baseDiv)};

            SDL_Texture* textTexture{SDL_CreateTextureFromSurface(renderer, textSurface)};

            SDL_RenderCopy(renderer, textTexture, &sourceRect, &targetRect);

            SDL_DestroyTexture(textTexture);
            SDL_FreeSurface(textSurface);

            // Counter will be rendered once per frame, outside the loop
        }
    }
}

void Selector::drawTitle(const string& title)
{
    int lineHeight = 40; // Height between lines (adjustable according to font size)
    int yOffset = 10;    // Vertical position of first line

    // Divide title with line breaks
    std::istringstream stream(title);
    string line;

    while (std::getline(stream, line, '\n'))
    {
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, line.c_str(), {255, 255, 255, 255});

        SDL_Rect sourceRect{0, 0, textSurface->w, textSurface->h};
        SDL_Rect targetRect{
            10, yOffset, textSurface->w / 3, textSurface->h / 3}; // Adjust text size here

        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

        SDL_RenderCopy(renderer, textTexture, &sourceRect, &targetRect);

        SDL_DestroyTexture(textTexture);
        SDL_FreeSurface(textSurface);

        yOffset += lineHeight; // Adjust offset for next line
    }
}

void Selector::drawSelector()
{
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 100);
    int windowWidth, windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);
    const float baseDiv = 5.0f; // Keep consistent with drawFileList scaling
    int rawLine = listFont ? TTF_FontLineSkip(listFont) : 100;
    if (rawLine <= 0) rawLine = 100;
    int lineAdvance = static_cast<int>(rawLine / baseDiv);
    if (lineAdvance < 12) lineAdvance = 12;
    int baseY = 500;
    SDL_Rect selectorRect{0, baseY, windowWidth, lineAdvance};
    SDL_RenderFillRect(renderer, &selectorRect);
}

void Selector::drawBackground()
{
    if (backgroundTexture)
        SDL_RenderCopy(renderer, backgroundTexture, NULL,
            NULL); // Full-window image
}

//          ╒═════════════════════════════════════════════════════════╕
//                                    PUBLIC
//          ╘═════════════════════════════════════════════════════════╛

Selector::Selector(string title, string backgroundImage, int listFontSize)
    : title(title)
    , backgroundTexture(nullptr)
    , listFont(nullptr)
    , counterFont(nullptr)
    , listFontSize(listFontSize)
    , chosenFileI(0)
{
#ifdef TRIMUI
    // Disable STDOUT to silent trimui's sdl logs
    int stdout_fd = dup(STDOUT_FILENO);
    freopen("/dev/null", "w", stdout);
#endif
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO);
    TTF_Init();

    // Load the "click" sound
    vec_string soundPaths = {"/mnt/SDCARD/System/usr/trimui/res/sound/click.wav", "click.wav",
        "/usr/trimui/res/sound/click.wav"};

    clickSound = loadClickSound(soundPaths);
    if (clickSound == nullptr)
        std::cerr << "No valid click sound file found.\n";

    window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800,
        1000, SDL_WINDOW_ALLOW_HIGHDPI);
    if (!window)
    {
        std::cerr << "Unable to create window" << '\n';
        std::exit(2);
    }

    // Prefer VSYNC to cap refresh and reduce CPU usage (fallback to default if it fails)
    renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer)
    {
        std::cerr << "VSYNC renderer failed: " << SDL_GetError() << "\nFalling back...\n";
        renderer = SDL_CreateRenderer(window, -1, 0);
        if (!renderer)
        {
            std::cerr << "Unable to create renderer" << '\n';
            std::exit(2);
        }
    }

#ifdef TRIMUI
    // Restore STDOUT
    fflush(stdout);
    dup2(stdout_fd, STDOUT_FILENO);
    close(stdout_fd);
#endif

    controller = NULL;
    if (SDL_NumJoysticks() > 0)
    {
        controller = SDL_GameControllerOpen(0);
        if (!controller)
            std::cerr << "Failed to open controller: " << SDL_GetError() << '\n';
    } else
    {
        std::cerr << "No joystick found\n";
    }

    // Load background image if supplied
    if (!backgroundImage.empty())
    {
        SDL_Surface* bgSurface = IMG_Load(backgroundImage.c_str());
        if (bgSurface)
        {
            backgroundTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);
            SDL_FreeSurface(bgSurface);
        } else
        {
            std::cerr << "Failed to load background image: " << IMG_GetError() << '\n';
        }
    }

    char buff[1024];
    ssize_t len = readlink("/proc/self/exe", buff, sizeof(buff) - 1);
    font_path = "";
    if (len != -1)
    {
        buff[len] = '\0';
        // Derive from executable directory without std::filesystem
        std::string path(buff);
        size_t pos = path.find_last_of('/');
        if (pos != std::string::npos)
            font_path = path.substr(0, pos + 1) + "Anonymous_Pro.ttf";
    }
    if (font_path.empty())
    {
        // Fallback to current working directory
        font_path = "Anonymous_Pro.ttf";
    }

    font = TTF_OpenFont(font_path.c_str(), 100);
    if (!font || font_path == "")
    {
        std::cerr << "Unable to open font file." << '\n';
        std::exit(2);
    }
    // Use provided listFontSize or default to 100 (old base size) if invalid/non-positive
    if (this->listFontSize <= 0) this->listFontSize = 100;
    listFont = TTF_OpenFont(font_path.c_str(), this->listFontSize);
    if (!listFont)
    {
        std::cerr << "Failed to load list font, falling back to title font size: "
                  << TTF_GetError() << std::endl;
        listFont = font; // fall back; do not close twice
    }
    // Create a small cached font for the counter
    counterFont = TTF_OpenFont(font_path.c_str(), 24);
    if (!counterFont)
        std::cerr << "Failed to load counter font: " << TTF_GetError() << std::endl;
}

Selector::~Selector()
{
    if (clickSound)
    {
        Mix_FreeChunk(clickSound);
        clickSound = nullptr;
    }

    if (font && font != listFont)
        TTF_CloseFont(font);
    if (listFont && listFont != font)
        TTF_CloseFont(listFont);
    if (counterFont)
        TTF_CloseFont(counterFont);
    if (backgroundTexture)
        SDL_DestroyTexture(backgroundTexture);
    if (controller)
        SDL_GameControllerClose(controller);
    if (renderer)
        SDL_DestroyRenderer(renderer);
    if (window)
        SDL_DestroyWindow(window);

    Mix_CloseAudio();
    Mix_Quit(); // Exit SDL_mixer
    TTF_Quit();
    SDL_Quit();
}

void Selector::setCustom(vec_string customChoices)
{
    chosenFileI = 0;
    fileList = customChoices; // Use the custom choices provided
    isCustom = 1;
    /*   drawTitle("Loading...");
      SDL_RenderPresent(renderer); */
}

void Selector::setFolder(string directory, bool recursive, vec_string filters)
{
    this->filters = filters;
    drawTitle("Loading...");
    SDL_RenderPresent(renderer);
    getFileList(directory, recursive); // Call with filters and recursive flag
    isCustom = 0;
}

int Selector::run()
{
    auto lastDpadPressTime = std::chrono::steady_clock::now();
    const int scrollIntervalMs = 220; // Adjust scrolling speed here
    bool dpadDownPressed = false;
    bool dpadUpPressed = false;
    bool needsRedraw = true; // Redraw only on changes

    while (1)
    {
        SDL_Event event;
        // Wait up to ~16ms for an event to reduce busy-waiting; then drain queue
        bool shouldExit = false;
        int exitCode = -1;
        if (SDL_WaitEventTimeout(&event, 16))
        {
            // Handle the event we waited for
            do
            {
                switch (event.type)
                {
                case SDL_QUIT:
                    shouldExit = true;
                    exitCode = -1;
                    break;
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym)
                    {
                    case SDLK_ESCAPE:
                        shouldExit = true;
                        exitCode = -1;
                        break;
                    case SDLK_DOWN:
                    case SDLK_s:
                        if (chosenFileI < static_cast<int>(fileList.size()) - 1)
                        {
                            ++chosenFileI;
                            if (clickSound) Mix_PlayChannel(-1, clickSound, 0);
                            needsRedraw = true;
                        }
                        break;
                    case SDLK_UP:
                    case SDLK_w:
                        if (chosenFileI > 0)
                        {
                            --chosenFileI;
                            if (clickSound) Mix_PlayChannel(-1, clickSound, 0);
                            needsRedraw = true;
                        }
                        break;
                    case SDLK_RETURN:
                        shouldExit = true;
                        exitCode = -1;
                        break;
                    }
                    break;
                case SDL_CONTROLLERBUTTONDOWN:
                    switch (event.cbutton.button)
                    {
                    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                        dpadDownPressed = true;
                        lastDpadPressTime = std::chrono::steady_clock::now();
                        if (chosenFileI < static_cast<int>(fileList.size()) - 1)
                        {
                            ++chosenFileI;
                            if (clickSound) Mix_PlayChannel(-1, clickSound, 0);
                        needsRedraw = true;
                        }
                        break;
                    case SDL_CONTROLLER_BUTTON_DPAD_UP:
                        dpadUpPressed = true;
                        lastDpadPressTime = std::chrono::steady_clock::now();
                        if (chosenFileI > 0)
                        {
                            --chosenFileI;
                            if (clickSound) Mix_PlayChannel(-1, clickSound, 0);
                        needsRedraw = true;
                        }
                        break;
                    case SDL_CONTROLLER_BUTTON_B:
                        shouldExit = true;
                        exitCode = 1;
                        break;
                    case SDL_CONTROLLER_BUTTON_A:
                        shouldExit = true;
                        exitCode = -1;
                        break;
                    case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: // L1
                        if (chosenFileI > 0)
                        {
                            chosenFileI -= filesPerPage;
                            if (chosenFileI < 0)
                                chosenFileI = 0; // Prevent underflow
                            if (clickSound) Mix_PlayChannel(-1, clickSound, 0);
                        needsRedraw = true;
                        }
                        break;
                    case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: // R1
                        if (chosenFileI < static_cast<int>(fileList.size()) - 1)
                        {
                            chosenFileI += filesPerPage;
                            if (chosenFileI >= static_cast<int>(fileList.size()))
                                chosenFileI = static_cast<int>(fileList.size()) - 1; // Prevent overflow
                            if (clickSound) Mix_PlayChannel(-1, clickSound, 0);
                        needsRedraw = true;
                        }
                        break;
                    }
                    break;
                case SDL_WINDOWEVENT:
                    // Redraw on expose, resize, etc.
                    needsRedraw = true;
                    break;
                case SDL_CONTROLLERBUTTONUP:
                    if (event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN)
                        dpadDownPressed = false;
                    else if (event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_UP)
                        dpadUpPressed = false;
                    break;
                }
            } while (SDL_PollEvent(&event));
        }
        auto currentTime = std::chrono::steady_clock::now();
        auto timeSinceLastPress =
            std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastDpadPressTime)
                .count();

        if (dpadDownPressed && timeSinceLastPress >= scrollIntervalMs)
        {
            lastDpadPressTime = currentTime;
            if (chosenFileI < static_cast<int>(fileList.size()) - 1)
            {
                ++chosenFileI;
        if (clickSound) Mix_PlayChannel(-1, clickSound, 0);
                needsRedraw = true;
            }
        } else if (dpadUpPressed && timeSinceLastPress >= scrollIntervalMs)
        {
            lastDpadPressTime = currentTime;
            if (chosenFileI > 0)
            {
                --chosenFileI;
        if (clickSound) Mix_PlayChannel(-1, clickSound, 0);
                needsRedraw = true;
            }
        }
        if (needsRedraw)
        {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            drawBackground();
            drawSelector();
            drawTitle(title);
            drawFileList();
            // Render selection counter once per frame
            renderCounter(renderer, chosenFileI + 1, static_cast<int>(fileList.size()));
            SDL_RenderPresent(renderer);
            needsRedraw = false;
        }
        if (shouldExit)
        {
            // Force a redraw before exit to ensure the last selection is visible
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            drawBackground();
            drawSelector();
            drawTitle(title);
            drawFileList();
            renderCounter(renderer, chosenFileI + 1, static_cast<int>(fileList.size()));
            SDL_RenderPresent(renderer);
            return exitCode;
        }
    }
}

string Selector::get()
{
    return chosenFileI < 0 ? "" : fileList[chosenFileI];
}
