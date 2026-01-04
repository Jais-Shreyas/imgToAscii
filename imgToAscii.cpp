#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <string>
#include "stb_image.h"

struct Dimensions
{
  int height;
  int width;
};

struct Arguments
{
  int newHeight;
  std::string mode;
  char *imagePath;
  char *outputPath;
};

#ifdef _WIN32 // for Windows systems
#include <windows.h>
Dimensions getTerminalHeightWidth()
{
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  int columns, rows;

  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
  columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
  rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

  return {rows, columns};
}
#else // for Unix-like systems such as Linux and macOS
#include <unistd.h>
#include <sys/ioctl.h>
Dimensions getTerminalHeightWidth()
{
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  return {w.ws_row, w.ws_col};
}
#endif

bool isInteger(const char *str)
{
  for (int i = 0; str[i] != '\0'; i++)
  {
    if (str[i] < '0' || str[i] > '9')
      return false;
  }
  return true;
}

Arguments parseArguments(int argc, char **argv)
{
  Arguments args;
  // default height is terminal height
  args.newHeight = 0; // 0 indicates to use terminal height
  // default mode is rgb
  args.mode = "rgb";

  // default output is terminal
  args.imagePath = nullptr;
  args.outputPath = nullptr;

  for (int i = 1; i < argc - 1; i++)
  {
    if (strcmp(argv[i], "-i") == 0)
    {
      args.imagePath = argv[++i];
    }
    else if (strcmp(argv[i], "-h") == 0)
    {
      if (isInteger(argv[i + 1]))
      {
        args.newHeight = atoi(argv[++i]);
        if (args.newHeight <= 0)
        {
          std::cout << "Height must be a positive integer\n";
          exit(1);
        }
      }
      else
      {
        std::cout << "Invalid height argument\n";
        std::cout << "Use -h <integer> or remove -h flag for using terminal height \n";
        exit(1);
      }
    }
    else if (strcmp(argv[i], "-m") == 0)
    {
      if (strcmp(argv[i + 1], "ascii") == 0 || strcmp(argv[i + 1], "rgb") == 0)
      {
        args.mode = argv[++i];
      }
      else
      {
        std::cout << "Invalid mode argument\n";
        std::cout << "Use -m ascii or -m rgb\n";
        exit(1);
      }
    }
    else if (strcmp(argv[i], "-o") == 0)
    {
      args.outputPath = argv[++i];
    }
  }

  if (args.imagePath == nullptr)
  {
    std::cout << "Image path not provided\n";
    std::cout << "Use -i <image_path> to provide input image path\n";
    std::cout.flush();
    exit(1);
  }

  return args;
}

// below code for RGB coloured output in terminal referenced from internet
void rgb(char text, int r, int g, int b, int br = -1, int bg = -1, int bb = -1)
{
  // background color
  if (br != -1 && bg != -1 && bb != -1)
  {
    std::cout << "\033[48;2;" << br << ";" << bg << ";" << bb << "m";
  }

  // foreground color
  if (text != ' ') // no sense of text color if text is space, saves printing time and space in output file
  {
    std::cout << "\033[38;2;" << r << ";" << g << ";" << b << "m";
  }

  std::cout << text;
}

const char asciiChars[] = "@%#*+=-:."; // characters from dense to sparse

int main(int argc, char **argv)
{
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  if (argc == 1 || (argc == 2 && strcmp(argv[1], "--help") == 0))
  {
    std::cout << "Usage:\n"
                 "  imgToAscii -i <image_path> [-h <new_height>] [-m <mode>] [-o <output_path>]\n\n"
                 "Options:\n"
                 "  -i <image_path>     Path to the input image (required)\n"
                 "  -h <new_height>     Output height in characters (optional, default: terminal height)\n"
                 "  -m <mode>           Output mode: ascii | rgb (optional, default: ascii)\n"
                 "  -o <output_path>    Write output to a file instead of stdout\n"
                 "  --help              Show this help message and exit\n";

    return 0;
  }

  Dimensions termSize = getTerminalHeightWidth();

  Arguments args = parseArguments(argc, argv);
  if (args.outputPath != nullptr)
  {
    freopen(args.outputPath, "w", stdout);
  }

  int width, height, channels;
  unsigned char *img = stbi_load(args.imagePath, &width, &height, &channels, 3);
  channels = 3;

  bool isAscii = (args.mode == "ascii");
  bool isRgb = (args.mode == "rgb");

  int newHeight = args.newHeight;
  int newWidth;
  if (newHeight == 0) // if newHeight is 0, use terminal height
  {
    newHeight = termSize.height;
    newWidth = width * newHeight / height * 2; // Default height to width ratio for terminal characters is 2:1

    if (newWidth > termSize.width) // if the new width exceeds terminal width, adjust both new width and new height
    {
      newWidth = termSize.width;
      newHeight = height * newWidth / (width * 2);
    }
  }
  else
  {
    newWidth = width * newHeight / height * 2; // Default height to width ratio for terminal characters is 2:1
  }

  for (int x = 0; x < newHeight; x++)
  {
    for (int y = 0; y < newWidth; y++)
    {
      int index = ((x * height / newHeight) * width + (y * width / newWidth)) * channels;

      int r = img[index + 0];
      int g = img[index + 1];
      int b = img[index + 2];

      if (isAscii)
      {
        int gray = (299 * r + 587 * g + 114 * b) / 1000;
        int charIndex = gray * sizeof(asciiChars) / 256;
        std::cout << asciiChars[charIndex];
      }
      else if (isRgb)
      {
        rgb(' ', 0, 0, 0, r, g, b); // background color set to pixel color, no need for foreground as of now, could change to only foreground with different characters
      }
      else
      {
        // should not reach here
      }
    }

    std::cout << "\033[0m"; // resetting color at the end of each line
    std::cout << "\n";
  }

  stbi_image_free(img);
  return 0;
}
