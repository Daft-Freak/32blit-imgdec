#include "imgdec.hpp"

#include "JPEGDEC.h"

using namespace blit;

struct DecodeData {
  Size size;
  uint8_t *data;
};

// format conversion

static int jpeg_draw_rgb888(JPEGDRAW *pDraw) {
  auto img = reinterpret_cast<DecodeData *>(pDraw->pUser);

  auto in = reinterpret_cast<uint32_t *>(pDraw->pPixels);
  auto out = img->data + (pDraw->x + pDraw->y * img->size.w) * 3;
  auto row_skip = (img->size.w - pDraw->iWidth) * 3;

  // 8888 -> 888, R/B swap
  for(int y = 0; y < pDraw->iHeight; y++) {
    for(int x = 0; x < pDraw->iWidth; x++) {
      auto in_pixel = *in++;
      *out++ = in_pixel >> 16;
      *out++ = in_pixel >> 8;
      *out++ = in_pixel;
    }
    out += row_skip;
  }

  return 1;
}

static int jpeg_draw_rgba8888(JPEGDRAW *pDraw) {
  auto img = reinterpret_cast<DecodeData *>(pDraw->pUser);

  auto in = reinterpret_cast<uint32_t *>(pDraw->pPixels);
  auto out = reinterpret_cast<uint32_t *>(img->data) + (pDraw->x + pDraw->y * img->size.w);
  auto row_skip = (img->size.w - pDraw->iWidth);

  // R/B swap
  for(int y = 0; y < pDraw->iHeight; y++) {
    for(int x = 0; x < pDraw->iWidth; x++) {
      auto pixel = __builtin_bswap32(*in++); // swap the whole thing
      pixel = pixel >> 8 | pixel << 24; // then rotate alpha back to the right end
      *out++ = pixel;
    }
    out += row_skip;
  }

  return 1;
}

static int jpeg_draw_rgba565(JPEGDRAW *pDraw) {
  auto img = reinterpret_cast<DecodeData *>(pDraw->pUser);

  auto in = pDraw->pPixels;
  auto out = reinterpret_cast<uint16_t *>(img->data) + (pDraw->x + pDraw->y * img->size.w);
  auto row_skip = (img->size.w - pDraw->iWidth);

  // R/B swap
  for(int y = 0; y < pDraw->iHeight; y++) {
    for(int x = 0; x < pDraw->iWidth; x++) {
      auto pixel = *in++;
      *out++ = (pixel & 0x07E0) | pixel >> 11 | pixel << 11;
    }
    out += row_skip;
  }

  return 1;
}

// file IO
static void *jpeg_file_open(const char *filename, int32_t *file_size) {
  auto fh = new File{filename};

  // check for success
  if(!fh->is_open()) {
    delete fh;
    return nullptr;
  }

  *file_size = fh->get_length();
  return fh;
}

static void jpeg_file_close(void *handle) {
  auto fh = reinterpret_cast<File *>(handle);
  delete fh;
}

static int32_t jpeg_file_read(JPEGFILE *file, uint8_t *buf, int32_t len) {
  auto fh = reinterpret_cast<File *>(file->fHandle);

  auto ret = fh->read(file->iPos, len, reinterpret_cast<char *>(buf));

  file->iPos += ret;

  return ret;
}

// the same as seekMem (32blit file api doesn't have seek)
static int32_t jpeg_file_seek(JPEGFILE *file, int32_t position) {
  if(position < 0)
    position = 0;
  else if(position >= file->iSize)
    position = file->iSize - 1;

  file->iPos = position;
  return position;
}

// helpers
static JPEG_DRAW_CALLBACK *format_to_draw_cb(PixelFormat format) {
  switch(format) {
    case PixelFormat::RGB:
      return jpeg_draw_rgb888;
    case PixelFormat::RGBA:
      return jpeg_draw_rgba8888;
    case PixelFormat::RGB565:
      return jpeg_draw_rgba565;
    default:
      return nullptr;
  }
}

static Surface *decode_to_surface(JPEGDEC &dec, PixelFormat format) {
  Size bounds(dec.getWidth(), dec.getHeight());

  dec.setPixelType(format == PixelFormat::RGB565 ? RGB565_LITTLE_ENDIAN : RGB8888);

  int bytes_per_pixel = pixel_format_stride[static_cast<int>(format)];
  auto data = new uint8_t[bounds.area() * bytes_per_pixel];
  DecodeData ret{bounds, data};

  dec.setUserPointer(&ret);

  if(!dec.decode(0, 0, 0)) {
    delete[] data;
    dec.close();
    return nullptr;
  }

  dec.close();
  return new Surface(data, format, bounds);
}

// main API
namespace imgdec {
  Surface *decode_jpeg_buffer(const uint8_t *ptr, uint32_t len, PixelFormat format) {
    JPEGDEC dec;

    auto cb = format_to_draw_cb(format);
    if(!cb) // unhandled format
      return nullptr;

    if(!dec.openFLASH(const_cast<uint8_t *>(ptr), len, cb))
      return nullptr;

    return decode_to_surface(dec, format);
  }

  Surface *decode_jpeg_file(const char *filename, PixelFormat format) {
    JPEGDEC dec;

    auto cb = format_to_draw_cb(format);
    if(!cb) // unhandled format
      return nullptr;

    if(!dec.open(filename, jpeg_file_open, jpeg_file_close, jpeg_file_read, jpeg_file_seek, cb))
      return nullptr;

    return decode_to_surface(dec, format);
  }
}

