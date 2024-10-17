#pragma once

#include "graphics/surface.hpp"

namespace imgdec {
  blit::Surface *decode_jpeg_buffer(const uint8_t *ptr, uint32_t len, blit::PixelFormat format);
}
