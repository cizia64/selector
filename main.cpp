#include "Selector.hpp"

#include <fstream>
#include <iostream>
#define HELP_MSG                                                                                   \
    "Usage: filechooser [options]\n"                                                               \
    "Options:\n"                                                                                   \
    "  -t <title>       Set a title to the selector.\n"                                            \
    "  -i <image>       Set a background image.\n"                                                 \
    "  -d <directory>   Set a directory to select files from.\n"                                   \
    "  -r               Use with directory. Add files from folder recursivly.\n"                   \
    "  -f <filter>      Use with directory. Must be last flag.\n"                                  \
    "  -c <choices> ... Get custom strings. Must be last flag.\n"                                  \
    "  -cf <file> ...   Get custom strings from a file lines.\n"                       \
    "  -fs <size>       Set list font size (pt). Default matches previous visual size.\n"

void customFromFile(string file, vec_string& customChoices)
{
    std::ifstream inputFile(file);
    if (!inputFile)
    {
        std::cerr << "Error: Cannot open file " << file << "\n";
        return;
    }
    string line;
    while (std::getline(inputFile, line))
        if (!line.empty())
            customChoices.push_back(line);
}

int main(int argc, char* argv[])
{
    vec_string arguments(argv, argv + argc);
    auto arg = arguments.begin();

    string title = FILECHOOSER_TITLE;
    string backgroundImage;
    string directory = ".";
    vec_string filters; // Container for filters
    bool recursive = false;
    vec_string customChoices;
    int listFontSize = -1; // negative means use default
    //
    auto unescapeString = [](const string &s) {
        string out;
        out.reserve(s.size());
        for (size_t i = 0; i < s.size(); ++i) {
            if (s[i] == '\\' && i + 1 < s.size()) {
                ++i;
                switch (s[i]) {
                case 'n': out.push_back('\n'); break;
                case 't': out.push_back('\t'); break;
                case '\\': out.push_back('\\'); break;
                case 'r': out.push_back('\r'); break;
                default: out.push_back(s[i]); break;
                }
            } else {
                out.push_back(s[i]);
            }
        }
        return out;
    };

    while (++arg < arguments.end())
    {
        if (*arg == "-t" && arg < arguments.end() - 1 &&
            (arg + 1)->length() > 0) // Set a title to the selector.
            title = unescapeString(*++arg);
        else if (*arg == "-i" && arg < arguments.end() - 1) // Set a background image.
            backgroundImage = *++arg;
        else if (*arg == "-d" && arg < arguments.end() - 1) // Set a directory to select files from.
            directory = *++arg;
        else if (*arg == "-r") // User with directory. Add files from folder recursivly.
            recursive = true;
        else if (*arg == "-f") // Use with directory. Must be last flag.
            while (arg < arguments.end() - 1)
                filters.push_back(*++arg);
        else if (*arg == "-c") // Get custom strings. Must be last flag.
            while (arg < arguments.end() - 1)
                customChoices.push_back(*++arg);
        else if (*arg == "-cf" &&
                 arg < arguments.end() - 1) // Get custom strings from a file lines.
            customFromFile(*++arg, customChoices);
        else if (*arg == "-fs" && arg < arguments.end() - 1)
        {
            try {
                listFontSize = std::stoi(*++arg);
            } catch (...) {
                std::cerr << "Invalid -fs value\n";
                return 1;
            }
        }
        else
        {
            std::cerr << HELP_MSG;
            return 1;
        }
    }

    Selector* selector = new Selector(title, backgroundImage, listFontSize);

    if (!customChoices.empty())
        selector->setCustom(customChoices);
    else
        selector->setFolder(directory, recursive, filters);

    if (selector->run() == -1)
    {
        std::cout << "No file selected\n";
    } else
    {
        string selected = selector->get();
        std::cout << "You selected: " << selected << "\n";
    }

    delete selector; // Clean up the dynamically allocated object

    return 0;
}
