#include "common.hpp"

std::vector<std::byte> readStream(std::istream& f, size_t start, long len){
    // Determine the length of data we will be reading
    f.seekg(0L, std::ios::end);
    if (long maxLen = size_t(f.tellg()) - start; len == -1 || len > maxLen) len = maxLen;
    f.clear();

    // Read from the specified start into the output vector
    f.seekg(start, std::ios::beg);
    std::vector<std::byte> out(len);
    f.readsome((char*) out.data(), len);

    return out;
}
