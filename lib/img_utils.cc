#include "img_utils.h"
#include "magick/image.h"
#include <climits>
#include <limits>

namespace rgb_matrix {

// Given the filename, load the image and scale to the size of the
// matrix.
// // If this is an animated image, the resutlting vector will contain multiple.
ImageVector LoadImageAndScaleImage(const char *filename, int target_width,
                                   int target_height) {
  ImageVector result;

  ImageVector frames;
  try {
    readImages(&frames, filename);
  } catch (std::exception &e) {
    if (e.what())
      fprintf(stderr, "%s\n", e.what());
    return result;
  }

  if (frames.empty()) {
    fprintf(stderr, "No image found.");
    return result;
  }

  // Animated images have partial frames that need to be put together
  if (frames.size() > 1) {
    Magick::coalesceImages(&result, frames.begin(), frames.end());
  } else {
    result.push_back(frames[0]); // just a single still image.
  }

  for (Magick::Image &image : result) {
    image.scale(Magick::Geometry(target_width, target_height));
  }

  return result;
}

// Copy an image to a Canvas. Note, the RGBMatrix is implementing the Canvas
// interface as well as the FrameCanvas we use in the double-buffering of the
// animted image.
void CopyImageToCanvas(const Magick::Image *image, Canvas *canvas, int offset_x,
                       int offset_y) {
  // Copy all the pixels to the canvas.
  for (size_t y = 0; y < image->rows(); ++y) {
    for (size_t x = 0; x < image->columns(); ++x) {
      const Magick::Color &c = image->pixelColor(x, y);
      if (c.alphaQuantum() < 256) {
        canvas->SetPixel(x + offset_x, y + offset_y,
                         ScaleQuantumToChar(c.redQuantum()),
                         ScaleQuantumToChar(c.greenQuantum()),
                         ScaleQuantumToChar(c.blueQuantum()));
      }
    }
  }
}

char scaleQuantumWithOpacity(Magick::Quantum quantum, char opacity) {
  return (char)((int)opacity * ScaleQuantumToChar(quantum)) / CHAR_MAX;
}
void CopyImageToCanvas(const Magick::Image *image, Canvas *canvas, int offset_x,
                       int offset_y, char opacity) {
  // Copy all the pixels to the canvas.
  for (size_t y = 0; y < image->rows(); ++y) {
    for (size_t x = 0; x < image->columns(); ++x) {
      const Magick::Color &c = image->pixelColor(x, y);
      // if (c.alphaQuantum() < 256)
      {
        canvas->SetPixel(x + offset_x, y + offset_y,
                         scaleQuantumWithOpacity(c.redQuantum(), opacity),
                         scaleQuantumWithOpacity(c.greenQuantum(), opacity),
                         scaleQuantumWithOpacity(c.blueQuantum(), opacity));
      }
    }
  }
}
} // namespace rgb_matrix
