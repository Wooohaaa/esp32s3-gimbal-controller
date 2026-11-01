#include "CoordinateFrameParser.h"

namespace {
constexpr uint8_t kHeader = 0x2C;
constexpr uint8_t kLength = 7;
constexpr uint8_t kFooter = 0x5B;
}

CoordinateFrameParser::CoordinateFrameParser() : state_(0), frame_{0} {}

void CoordinateFrameParser::beginFrame() {
    frame_[0] = kHeader;
    state_ = 1;
}

void CoordinateFrameParser::reset() {
    state_ = 0;
    for (uint8_t &byte : frame_) byte = 0;
}

bool CoordinateFrameParser::push(uint8_t value) {
    switch (state_) {
    case 0:
        if (value == kHeader) beginFrame();
        break;
    case 1:
        if (value == kLength) {
            frame_[1] = value;
            state_ = 2;
        } else if (value == kHeader) {
            beginFrame();
        } else {
            reset();
        }
        break;
    case 2:
        frame_[2] = value;
        state_ = 3;
        break;
    case 3:
        frame_[3] = value;
        state_ = 4;
        break;
    case 4:
        if (value == 3) {
            frame_[4] = value;
            state_ = 5;
        } else if (value == kHeader) {
            beginFrame();
        } else {
            reset();
        }
        break;
    case 5:
        if (value == 4) {
            frame_[5] = value;
            state_ = 6;
        } else if (value == kHeader) {
            beginFrame();
        } else {
            reset();
        }
        break;
    case 6:
        if (value == kFooter) {
            frame_[6] = value;
            state_ = 0;
            return true;
        }
        if (value == kHeader) beginFrame();
        else reset();
        break;
    default:
        reset();
        break;
    }
    return false;
}

uint8_t CoordinateFrameParser::x() const { return frame_[2]; }
uint8_t CoordinateFrameParser::y() const { return frame_[3]; }
