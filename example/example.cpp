#include <cstring>

#include "32blit.hpp"
#include "imgdec.hpp"

#include "assets.hpp"

using namespace blit;

static Surface *image_surface;

// blit doesn't support 565 src
// this does a raw copy from a surface of a matching format
static void raw_copy(Surface *dest, Surface *src, Point pos) {
    // assuming pos is inside clip rect...

    Rect dest_rect{pos, src->bounds};
    dest_rect = dest_rect.intersection(dest->clip);

    for(int y = 0; y < dest_rect.h; y++) {
        auto dest_ptr = dest->ptr(dest_rect.x, dest_rect.y + y);
        auto src_ptr = src->ptr(0, y);
        memcpy(dest_ptr, src_ptr, dest_rect.w * dest->pixel_stride);
    }
}

void init() {
    set_screen_mode(ScreenMode::hires, PixelFormat::RGB565);

    // make assets into files for convenience
    File::add_buffer_file("image.bmp", asset_image_bmp, asset_image_bmp_length);
    File::add_buffer_file("image.jpeg", asset_image_jpeg, asset_image_jpeg_length);

    // load one of them
    // note that because BMP loading uses the built-in loading in Surface, the format is ignored
    // (and RGB565 isn't supported)
    image_surface = imgdec::decode_file("image.jpeg", PixelFormat::RGB565);

    // could also use for a specific format
    // image_surface = imgdec::decode_jpeg_buffer(asset_image_jpeg, asset_image_jpeg_length, PixelFormat::RGB565);
}

void render(uint32_t time) {
    screen.pen = {0, 0, 0};
    screen.clear();

    if(image_surface) {
        if(image_surface->format != PixelFormat::RGB565)
            screen.blit(image_surface, {{0, 0}, image_surface->bounds}, {0, 0});
        else
            raw_copy(&screen, image_surface, {0, 0});
    }
}

void update(uint32_t time) {
}