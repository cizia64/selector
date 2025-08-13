# SDL Selector Application

Selector is a selection tool built for CrossMix-OS with SDL, designed to offer an intuitive and customizable interface for users to make selections from either files list or from user-specified options.

With support for custom title, background image, and recursive file searching, this application provides a clean, navigable interface using both keyboard or game controller inputs. 


## Usage

To launch the application, use the following command-line options to customize its behavior:

    ./selector [options]

## Command-Line Options

- **`-d <directory>`**  
  Specifies the directory to list files from. The application will display all files in this directory. If omitted, the application defaults to the current directory.

- **`-t <title>`**  
  Sets the title displayed at the top of the selection window. You can include line breaks in the title by using `\n` in the string. Example: `"Select a file\nPress Enter to confirm"`.

- **`-i <image_path>`**  
  Provides a path to a background image file. This image will be displayed as the background of the selection window. Ensure the image file format is supported by SDL_image (e.g., PNG, JPEG).

- **`-r`**  
  Enables recursive file listing. If used with the `-d` option, the application will search through subdirectories for files as well.

- **`-c <option1> <option2> ...`**  
  Allows you to specify a custom list of options instead of listing files. These options are displayed in the selection window for the user to choose from.
  ( Must be the last used flags )

## Interactive Controls

- **Arrow Keys / D-Pad**  
  Navigate through the list of files or options.

- **L1/L2**
  Jump 10 items backward/forward in the list

- **Enter / A Button**  
  Select the highlighted item.

- **Escape / B Button**  
  Exit the application.

## Examples

1. **List Files in Current Directory**

   `./selector`

2. **List Files in a Specific Directory**

   `./selector -d /path/to/directory`

3. **Custom Title with Line Breaks**

   `./selector -t "Choose a file\nUse arrows to navigate\nPress Enter to select"`

4. **With Background Image**

   `./selector -i /path/to/background.png`

5. **Recursive File Listing**

   `./selector -d /path/to/directory -r`

6. **Custom Options List**

   `./selector -t "Select an Option" -i /path/to/background.png -c "Option 1" "Option 2" "Option 3"`

## Notes

- Ensure that SDL, SDL_image, and SDL_ttf libraries are properly installed and configured for the application to run smoothly.
- The custom list feature (`-c`) can be used with -i or -t but can' be used with file listing options (`-d` and `-r`).

https://github.com/cizia64/selector

## Help the CrossMix-OS project

Contribute to this repo by making a Pull Request. If you have an improvement to propose and don't know how to use GitHub, send me a message!

Feel free to reach out to me to report bugs, request features, or just chat on **[Discord](https://discord.gg/Jd2azKX)** or on **[Github Issues](https://github.com/cizia64/CrossMix-OS/issues?q=is%3Aissue+is%3Aopen+sort%3Aupdated-desc)**

If you enjoy my work and want to support the countless hours/days invested on CrossMix-OS and its tools, here are my sponsors:

- [![Patreon](_assets/readme/patreon.png)](https://patreon.com/Cizia)
- [![Buy Me a Coffee](_assets/readme/bmc.png)](https://www.buymeacoffee.com/cizia)
- [![ko-fi](_assets/readme/ko-fi.png)](https://ko-fi.com/H2H7YPH3H)


