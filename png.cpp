#include "imgdec.hpp"

#include "PNGdec.h"

using namespace blit;

struct DecodeData {
  Size size;
  uint8_t *data;
  PNG *png;
};

// format conversion

static int png_draw_rgb888(PNGDRAW *pDraw) {
  auto img = reinterpret_cast<DecodeData *>(pDraw->pUser);

  auto out = img->data + (pDraw->y * img->size.w) * 3;

  switch(pDraw->iPixelType) {
    case PNG_PIXEL_GRAYSCALE:
      for(int x = 0; x < pDraw->iWidth; x++) {
        auto g = pDraw->pPixels[x];
        *out++ = g;
        *out++ = g;
        *out++ = g;
      }
      break;

    case PNG_PIXEL_TRUECOLOR:
      memcpy(out, pDraw->pPixels, pDraw->iWidth * 3);
      break;

    case PNG_PIXEL_INDEXED: {
      auto pal = pDraw->pPalette;
      for(int x = 0; x < pDraw->iWidth; x++) {
        auto i = pDraw->pPixels[x];
        *out++ = pal[i * 3 + 0];
        *out++ = pal[i * 3 + 1];
        *out++ = pal[i * 3 + 2];
      }
      break;
    }

    case PNG_PIXEL_GRAY_ALPHA:
      for(int x = 0; x < pDraw->iWidth; x++) {
        auto g = pDraw->pPixels[x * 2];
        *out++ = g;
        *out++ = g;
        *out++ = g;
      }
      break;

    case PNG_PIXEL_TRUECOLOR_ALPHA:
      for(int x = 0; x < pDraw->iWidth; x++) {
        *out++ = pDraw->pPixels[x * 4 + 0];
        *out++ = pDraw->pPixels[x * 4 + 1];
        *out++ = pDraw->pPixels[x * 4 + 2];
      }
      break;
  }

  return 1;
}

static int png_draw_rgba8888(PNGDRAW *pDraw) {
  auto img = reinterpret_cast<DecodeData *>(pDraw->pUser);

  auto out = img->data + (pDraw->y * img->size.w) * 4;

  switch(pDraw->iPixelType) {
    case PNG_PIXEL_GRAYSCALE:
      for(int x = 0; x < pDraw->iWidth; x++) {
        auto g = pDraw->pPixels[x];
        *out++ = g;
        *out++ = g;
        *out++ = g;
        *out++ = 255;
      }
      break;

    case PNG_PIXEL_TRUECOLOR:
      for(int x = 0; x < pDraw->iWidth; x++) {
        *out++ = pDraw->pPixels[x * 3 + 0];
        *out++ = pDraw->pPixels[x * 3 + 1];
        *out++ = pDraw->pPixels[x * 3 + 2];
        *out++ = 255;
      }
      break;

    case PNG_PIXEL_INDEXED: {
      auto pal = pDraw->pPalette;
      for(int x = 0; x < pDraw->iWidth; x++) {
        auto i = pDraw->pPixels[x];
        *out++ = pal[i * 3 + 0];
        *out++ = pal[i * 3 + 1];
        *out++ = pal[i * 3 + 2];
        *out++ = pDraw->iHasAlpha ? pal[768 + i] : 255;
      }
      break;
    }

    case PNG_PIXEL_GRAY_ALPHA:
      for(int x = 0; x < pDraw->iWidth; x++) {
        auto g = pDraw->pPixels[x * 2];
        *out++ = g;
        *out++ = g;
        *out++ = g;
        *out++ = pDraw->pPixels[x * 2 + 1];
      }
      break;

    case PNG_PIXEL_TRUECOLOR_ALPHA:
      memcpy(out, pDraw->pPixels, pDraw->iWidth * 4);
      break;
  }

  return 1;
}

static int png_draw_rgba565(PNGDRAW *pDraw) {
  auto img = reinterpret_cast<DecodeData *>(pDraw->pUser);

  auto out = reinterpret_cast<uint16_t *>(img->data) + (pDraw->y * img->size.w);

  img->png->getLineAsRGB565(pDraw, out, PNG_RGB565_LITTLE_ENDIAN, -1);

  // R/B swap
  for(int x = 0; x < pDraw->iWidth; x++) {
    auto pixel = *out;
    *out++ = (pixel & 0x07E0) | pixel >> 11 | pixel << 11;
  }

  return 1;
}

static int png_draw_palette(PNGDRAW *pDraw) {
  auto img = reinterpret_cast<DecodeData *>(pDraw->pUser);

  auto in = pDraw->pPixels;
  auto out = img->data + (pDraw->y * img->size.w);

  memcpy(out, in, pDraw->iWidth);
  return 1;
}


// file IO
// TODO: identical to JPEG
static void *png_file_open(const char *filename, int32_t *file_size) {
  auto fh = new File{filename};

  // check for success
  if(!fh->is_open()) {
    delete fh;
    return nullptr;
  }

  *file_size = fh->get_length();
  return fh;
}

static void png_file_close(void *handle) {
  auto fh = reinterpret_cast<File *>(handle);
  delete fh;
}

static int32_t png_file_read(PNGFILE *file, uint8_t *buf, int32_t len) {
  auto fh = reinterpret_cast<File *>(file->fHandle);

  auto ret = fh->read(file->iPos, len, reinterpret_cast<char *>(buf));

  file->iPos += ret;

  return ret;
}

// the same as seekMem (32blit file api doesn't have seek)
static int32_t png_file_seek(PNGFILE *file, int32_t position) {
  if(position < 0)
    position = 0;
  else if(position >= file->iSize)
    position = file->iSize - 1;

  file->iPos = position;
  return position;
}

// helpers
static PNG_DRAW_CALLBACK *format_to_draw_cb(PixelFormat format) {
  switch(format) {
    case PixelFormat::RGB:
      return png_draw_rgb888;
    case PixelFormat::RGBA:
      return png_draw_rgba8888;
    case PixelFormat::RGB565:
      return png_draw_rgba565;
    case PixelFormat::P:
      return png_draw_palette;
    default:
      return nullptr;
  }
}

static Surface *decode_to_surface(PNG &dec, PixelFormat format) {
  Size bounds(dec.getWidth(), dec.getHeight());

  if(dec.getBpp() != 8)
    return nullptr;

  // can only load indexed or grey images as paletted
  if(format == PixelFormat::P) {
    auto png_type = dec.getPixelType();
    if((png_type != PNG_PIXEL_GRAYSCALE && png_type != PNG_PIXEL_INDEXED))
      return nullptr;
  }

  int bytes_per_pixel = pixel_format_stride[static_cast<int>(format)];
  auto data = new uint8_t[bounds.area() * bytes_per_pixel];
  DecodeData dec_data{bounds, data, &dec};

  if(dec.decode(&dec_data, 0) != PNG_SUCCESS) {
    delete[] data;
    dec.close();
    return nullptr;
  }

  dec.close();
  auto ret = new Surface(data, format, bounds);

  // fill in palette
  if(format == PixelFormat::P) {
    ret->palette = new Pen[256];

    if(dec.getPixelType() == PNG_PIXEL_GRAYSCALE) {
      // greyscale palette
      for(int i = 0; i < 256; i++)
        ret->palette[i] = {i, i, i, 255};
    } else {
      auto png_pal = dec.getPalette();

      for(int i = 0; i < 256; i++)
        ret->palette[i] = {png_pal[i * 3 + 0], png_pal[i * 3 + 1], png_pal[i * 3 + 2], 255};

      // copy alpha if present
      if(dec.hasAlpha()) {
        for(int i = 0; i < 256; i++)
          ret->palette[i].a = png_pal[768 + i];
      }
    }
  }

  return ret;
}

// main API
namespace imgdec {
  Surface *decode_png_buffer(const uint8_t *ptr, uint32_t len, PixelFormat format) {
    PNG dec;

    auto cb = format_to_draw_cb(format);
    if(!cb) // unhandled format
      return nullptr;

    if(dec.openFLASH(const_cast<uint8_t *>(ptr), len, cb) != PNG_SUCCESS)
      return nullptr;

    return decode_to_surface(dec, format);
  }

  Surface *decode_png_file(const char *filename, PixelFormat format) {
    PNG dec;

    auto cb = format_to_draw_cb(format);
    if(!cb) // unhandled format
      return nullptr;

    if(dec.open(filename, png_file_open, png_file_close, png_file_read, png_file_seek, cb) != PNG_SUCCESS)
      return nullptr;

    return decode_to_surface(dec, format);
  }
}

