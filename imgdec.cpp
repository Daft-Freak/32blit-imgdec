#include "imgdec.hpp"

using namespace blit;

namespace imgdec {
  Surface *decode_file(const char *filename, PixelFormat format) {
    // get the extension
    std::string_view sv(filename);
    auto last_dot = sv.find_last_of('.');
    auto ext = last_dot == std::string::npos ? "" : std::string(sv.substr(last_dot + 1));

    // lowercase it
    for(auto &c : ext)
      c = tolower(c);

    if(ext == "jpeg")
      return decode_jpeg_file(filename, format);

    return nullptr;
  }
}