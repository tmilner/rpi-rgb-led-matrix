#include <Magick++.h>
#include <magick/image.h>
#include "canvas.h"

namespace rgb_matrix
{
    using ImageVector = std::vector<Magick::Image>;

    ImageVector LoadImageAndScaleImage(const char *filename,
                                              int target_width,
                                              int target_height);

    void CopyImageToCanvas(const Magick::Image *image, Canvas *canvas, int offset_x, int offset_y);
}