#pragma once

#include <stdint.h>

class CoordinateFrameParser {
public:
    CoordinateFrameParser();
    bool push(uint8_t value);
    uint8_t x() const;
    uint8_t y() const;
    void reset();

private:
    void beginFrame();
    uint8_t state_;
    uint8_t frame_[7];
};
