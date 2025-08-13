#include "Selector.hpp"

#include <cmath>
#include <filesystem>
#include <iostream>
#include <stdint.h>
#include <unistd.h>

namespace fs = std::filesystem;

void Selector::renderCounter(SDL_Renderer* renderer, int currentOption, int totalOptions)
{
    // Define text color (white for example)
    SDL_Color textColor = {255, 255, 255, 255}; // RGBA format: white

    // Load a smaller font for the counter (e.g., size 12)
    TTF_Font* smallFont = TTF_OpenFont(font_path.c_str(),
        24); // Adjust the path and size as needed
    if (smallFont == nullptr)
    {
        std::cerr << "Failed to load small font: " << TTF_GetError() << std::endl;
        return;
    }

    // Create the counter text: "currentOption/totalOptions"
    string counterText = std::to_string(currentOption) + "/" + std::to_string(totalOptions);

    // Render the text into an SDL_Surface using the smaller font
    SDL_Surface* textSurface = TTF_RenderText_Solid(smallFont, counterText.c_str(), textColor);
    if (textSurface == nullptr)
    {
        std::cerr << "Failed to create text surface: " << TTF_GetError() << std::endl;
        TTF_CloseFont(smallFont); // Clean up the small font
        return;
    }

    // Convert the SDL_Surface to SDL_Texture
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (textTexture == nullptr)
    {
        std::cerr << "Failed to create texture: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(textSurface);
        TTF_CloseFont(smallFont); // Clean up the small font
        return;
    }

    // Get window size to position the counter in the bottom right corner
    int windowWidth, windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);

    // Get text width and height from the surface
    int textWidth = textSurface->w;
    int textHeight = textSurface->h;

    // Define destination rectangle for rendering the counter (bottom-right
    // corner)
    SDL_Rect dstRect = {
        windowWidth - textWidth - 10, windowHeight - textHeight - 10, textWidth, textHeight};

    // Render the text texture onto the screen
    SDL_RenderCopy(renderer, textTexture, nullptr, &dstRect);

    // Clean up
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
    TTF_CloseFont(smallFont); // Clean up the small font
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

void Selector::getFileList(string directory, bool recursive)
{
    auto matchFilters = [&](const std::filesystem::path& file) {
        // Convert file name to lowercase
        string filename = file.filename().string();
        std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);

        if (filters.empty())
            return true; // No filters means all files are allowed

        for (const auto& filter : filters)
        {
            string lowerFilter = filter;
            std::transform(lowerFilter.begin(), lowerFilter.end(), lowerFilter.begin(), ::tolower);

            // Check if the filter is present in the file name
            if (filename.find(lowerFilter) != string::npos)
                return true;
        }
        return false;
    };

    if (recursive)
    {
        auto files{std::filesystem::recursive_directory_iterator{
            directory, std::filesystem::directory_options::skip_permission_denied}};

        for (auto& file : files)
        {
            if (std::filesystem::is_regular_file(file) && matchFilters(file.path()))
                fileList.push_back(file.path().string());
        }
    } else
    {
        auto files{std::filesystem::directory_iterator{
            directory, std::filesystem::directory_options::skip_permission_denied}};

        for (auto& file : files)
        {
            if (std::filesystem::is_regular_file(file) && matchFilters(file.path()))
                fileList.push_back(file.path().string());
        }
    }

    std::sort(fileList.begin(), fileList.end());
}
void Selector::drawFileList()
{
    for (int i = 0; i < static_cast<int>(fileList.size()); ++i)
    {
        int y = 500 - chosenFileI * 30 + i * 30;

        if (y < 1000 && y > 0)
        {
            SDL_Surface* textSurface = TTF_RenderText_Blended(font, fileList[i].c_str(),
                {255, 255, 255, static_cast<uint8_t>(255 - std::abs(500 - y) / 2)});

            SDL_Rect sourceRect{0, 0, textSurface->w, textSurface->h};
            SDL_Rect targetRect{0, y, textSurface->w / 5, textSurface->h / 5};

            SDL_Texture* textTexture{SDL_CreateTextureFromSurface(renderer, textSurface)};

            SDL_RenderCopy(renderer, textTexture, &sourceRect, &targetRect);

            SDL_DestroyTexture(textTexture);
            SDL_FreeSurface(textSurface);

            // Call the method to display the counter in the bottom right corner
            renderCounter(renderer, chosenFileI + 1, static_cast<int>(fileList.size()));
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
    SDL_Rect selectorRect{0, 500, 800, 25};
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

Selector::Selector(string title, string backgroundImage)
    : title(title)
    , backgroundTexture(nullptr)
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

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer)
    {
        std::cerr << "Unable to create renderer" << '\n';
        std::exit(2);
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
        fs::path bin_folder = std::filesystem::path(buff).parent_path();
        fs::path font = "Anonymous_Pro.ttf";
        font_path = (bin_folder / font).string();
    }

    font = TTF_OpenFont(font_path.c_str(), 100);
    if (!font || font_path == "")
    {
        std::cerr << "Unable to open font file." << '\n';
        std::exit(2);
    }
}

Selector::~Selector()
{
    if (clickSound)
    {
        Mix_FreeChunk(clickSound);
        clickSound = nullptr;
    }

    if (font)
        TTF_CloseFont(font);
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

    while (1)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                return -1;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    return -1;
                case SDLK_DOWN:
                case SDLK_s:
                    if (chosenFileI < static_cast<int>(fileList.size()) - 1)
                    {
                        ++chosenFileI;
                        Mix_PlayChannel(-1, clickSound, 0);
                    }
                    break;
                case SDLK_UP:
                case SDLK_w:
                    if (chosenFileI > 0)
                    {
                        --chosenFileI;
                        Mix_PlayChannel(-1, clickSound, 0);
                    }
                    break;
                case SDLK_RETURN:
                    return -1;
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
                        Mix_PlayChannel(-1, clickSound, 0);
                    }
                    break;
                case SDL_CONTROLLER_BUTTON_DPAD_UP:
                    dpadUpPressed = true;
                    lastDpadPressTime = std::chrono::steady_clock::now();
                    if (chosenFileI > 0)
                    {
                        --chosenFileI;
                        Mix_PlayChannel(-1, clickSound, 0);
                    }
                    break;
                case SDL_CONTROLLER_BUTTON_B:
                    return 1;
                case SDL_CONTROLLER_BUTTON_A:
                    return -1;
                case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: // L1
                    if (chosenFileI > 0)
                    {
                        chosenFileI -= filesPerPage;
                        if (chosenFileI < 0)
                            chosenFileI = 0; // Prevent underflow
                        Mix_PlayChannel(-1, clickSound, 0);
                    }
                    break;
                case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: // R1
                    if (chosenFileI < static_cast<int>(fileList.size()) - 1)
                    {
                        chosenFileI += filesPerPage;
                        if (chosenFileI >= static_cast<int>(fileList.size()))
                            chosenFileI = static_cast<int>(fileList.size()) - 1; // Prevent overflow
                        Mix_PlayChannel(-1, clickSound, 0);
                    }
                    break;
                }
                break;
            case SDL_CONTROLLERBUTTONUP:
                if (event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN)
                    dpadDownPressed = false;
                else if (event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_UP)
                    dpadUpPressed = false;
                break;
            }
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
                Mix_PlayChannel(-1, clickSound, 0);
            }
        } else if (dpadUpPressed && timeSinceLastPress >= scrollIntervalMs)
        {
            lastDpadPressTime = currentTime;
            if (chosenFileI > 0)
            {
                --chosenFileI;
                Mix_PlayChannel(-1, clickSound, 0);
            }
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        drawBackground();
        drawSelector();
        drawTitle(title);
        drawFileList();
        SDL_RenderPresent(renderer);
        SDL_Delay(20);
    }
}

string Selector::get()
{
    return chosenFileI < 0 ? "" : fileList[chosenFileI];
}
