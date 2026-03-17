#include <cstring>

#include "32blit.hpp"
#include "imgdec.hpp"

#include "assets.hpp"

using namespace blit;

static Surface *jpeg_surface;

// blit only works from RGBA or palette surfaces...
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

    jpeg_surface = imgdec::decode_jpeg_buffer(asset_image_jpeg, asset_image_jpeg_length, PixelFormat::RGB565);
}

void render(uint32_t time) {
    screen.pen = {0, 0, 0};
    screen.clear();

    if(jpeg_surface)
        raw_copy(&screen, jpeg_surface, {0, 0});
}

void update(uint32_t time) {
}