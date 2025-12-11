#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
using std::cout;

// below code for RGB coloured output in terminal referenced from internet
void rgb(char text, int r, int g, int b, int br = -1, int bg = -1, int bb = -1)
{
  // background color
  if (br != -1 && bg != -1 && bb != -1)
  {
    cout << "\033[48;2;" << br << ";" << bg << ";" << bb << "m";
  }

  // foreground color
  cout << "\033[38;2;" << r << ";" << g << ";" << b << "m";

  cout << text << "\033[0m";
}

const char asciiChars[] = "@%#*+=-:."; // characters from dense to sparse

int main(int argc, char **argv)
{
  // Argument 1: new height
  // Argument 2: mode (-ascii / -rgb)
  // Argument 3: image path

  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  if (argc != 4)
  {
    cout << "\033[1;38;2;255;70;70m"
         << "Usage:\n"
         << "\033[0m"
         << "Arguments:  <height> <mode> <image>\n"
         << "e.g., ./imgToAscii 40 -ascii public/sample.jpg\n\n"
         << "Arguments Description:\n"
         << "  <height>   Output ASCII height\n"
         << "  <mode>     `-ascii` | `-rgb`\n"
         << "  <image>    Input image path (absolute / relative)\n";

    return 1;
  }

  int newHeight = atoi(argv[1]);
  char *mode = argv[2];
  char *imagePath = argv[3];

  int width, height, channels;
  unsigned char *img = stbi_load(imagePath, &width, &height, &channels, 3);
  channels = 3;

  if (img == nullptr)
  {
    cout << "\033[1;38;2;255;70;70m"
         << "Failed to load image\n"
         << "Try again with path in double quotes \"\""
         << "\033[0m";
    return 1;
  }

  int newWidth = width * newHeight / height * 2; // Default height to width ratio for terminal characters is 2:1
  bool isAscii = (strcmp(mode, "-ascii") == 0);
  bool isRgb = (strcmp(mode, "-rgb") == 0);
  if (!isAscii && !isRgb)
  {
    cout << "Invalid mode. Use -ascii or -rgb\n";
    stbi_image_free(img);
    return 1;
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
        cout << asciiChars[charIndex];
      }
      else // if (isRgb)
      {
        rgb(' ', 0, 0, 0, r, g, b); // background color set to pixel color, no need for foreground as of now, could change to only foreground with different characters
      }
    }
    cout << "\n";
  }

  stbi_image_free(img);
  return 0;
}