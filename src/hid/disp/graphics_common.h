#pragma once

#include <cstdint>

namespace daisy
{
/** Justifications */
enum class Alignment
{
    centered,
    topLeft,
    topCentered,
    topRight,
    bottomLeft,
    bottomCentered,
    bottomRight,
    centeredLeft,
    centeredRight
};

class Rectangle
{
  public:
    Rectangle() : x_(0), y_(0), width_(0), height_(0) {}

    Rectangle(int16_t width, int16_t height)
    : x_(0), y_(0), width_(max(0, width)), height_(max(0, height))
    {
    }

    Rectangle(int16_t x, int16_t y, int16_t width, int16_t height)
    : x_(x), y_(y), width_(max(0, width)), height_(max(0, height))
    {
    }

    Rectangle(const Rectangle& other) { *this = other; }

    Rectangle& operator=(const Rectangle& other)
    {
        x_      = other.x_;
        y_      = other.y_;
        width_  = other.width_;
        height_ = other.height_;
        return *this;
    }

    bool operator==(const Rectangle& other) const
    {
        return (x_ == other.x_) && (y_ == other.y_) && (width_ == other.width_)
               && (height_ == other.height_);
    }
    bool operator!=(const Rectangle& other) const { return !(*this == other); }

    bool IsEmpty() const { return (width_ <= 0) || (height_ <= 0); }

    int16_t GetX() const { return x_; }
    int16_t GetY() const { return y_; }
    int16_t GetWidth() const { return width_; }
    int16_t GetHeight() const { return height_; }
    int16_t GetRight() const { return x_ + width_; }
    int16_t GetBottom() const { return y_ + height_; }
    int16_t GetCenterX() const { return x_ + width_ / 2; }
    int16_t GetCenterY() const { return y_ + height_ / 2; }

    Rectangle WithX(int16_t x) const { return {x, y_, width_, height_}; }
    Rectangle WithY(int16_t y) const { return {x_, y, width_, height_}; }
    Rectangle WithWidth(int16_t width) const
    {
        return {x_, y_, width, height_};
    }
    Rectangle WithHeight(int16_t height) const
    {
        return {x_, y_, width_, height};
    }
    Rectangle WithSize(int16_t width, int16_t height) const
    {
        return {x_, y_, width, height};
    }
    Rectangle WithWidthKeepingCenter(int16_t width) const
    {
        return Rectangle(x_, y_, width, height_)
            .WithCenter(GetCenterX(), GetCenterY());
    }
    Rectangle WithHeightKeepingCenter(int16_t height) const
    {
        return Rectangle(x_, y_, width_, height)
            .WithCenter(GetCenterX(), GetCenterY());
    }
    Rectangle WithSizeKeepingCenter(int16_t width, int16_t height) const
    {
        return Rectangle(x_, y_, width, height)
            .WithCenter(GetCenterX(), GetCenterY());
    }

    Rectangle Reduced(int16_t sizeToReduce) const
    {
        return {int16_t(x_ + sizeToReduce),
                int16_t(y_ + sizeToReduce),
                int16_t(width_ - 2 * sizeToReduce),
                int16_t(height_ - 2 * sizeToReduce)};
    }
    Rectangle Reduced(int16_t xToReduce, int16_t yToReduce) const
    {
        return {int16_t(x_ + xToReduce),
                int16_t(y_ + yToReduce),
                int16_t(width_ - 2 * xToReduce),
                int16_t(height_ - 2 * yToReduce)};
    }

    Rectangle Translated(int16_t x, int16_t y) const
    {
        return {int16_t(x_ + x), int16_t(y_ + y), width_, height_};
    }

    Rectangle WithLeft(int16_t left) const
    {
        const auto newWidth = int16_t((x_ - left) + width_);
        return {left, y_, newWidth, height_};
    }

    Rectangle WithRight(int16_t right) const
    {
        const auto newWidth = int16_t(right - x_);
        return {x_, y_, newWidth, height_};
    }

    Rectangle WithTop(int16_t top) const
    {
        const auto newHeight = int16_t((y_ - top) + height_);
        return {x_, top, width_, newHeight};
    }

    Rectangle WithBottom(int16_t bottom) const
    {
        const auto newHeight = int16_t(bottom - y_);
        return {x_, y_, width_, newHeight};
    }

    Rectangle WithTrimmedLeft(int16_t pxToTrim) const
    {
        return {
            int16_t(x_ + pxToTrim), y_, int16_t(width_ - pxToTrim), height_};
    }

    Rectangle WithTrimmedRight(int16_t pxToTrim) const
    {
        return {x_, y_, int16_t(width_ - pxToTrim), height_};
    }

    Rectangle WithTrimmedTop(int16_t pxToTrim) const
    {
        return {
            x_, int16_t(y_ + pxToTrim), width_, int16_t(height_ - pxToTrim)};
    }

    Rectangle WithTrimmedBottom(int16_t pxToTrim) const
    {
        return {x_, y_, width_, int16_t(height_ - pxToTrim)};
    }

    Rectangle WithCenterX(int16_t centerX) const
    {
        return {int16_t(x_ + centerX - GetCenterX()), y_, width_, height_};
    }

    Rectangle WithCenterY(int16_t centerY) const
    {
        return {x_, int16_t(y_ + centerY - GetCenterY()), width_, height_};
    }

    Rectangle WithCenter(int16_t centerX, int16_t centerY) const
    {
        return {int16_t(x_ + centerX - GetCenterX()),
                int16_t(y_ + centerY - GetCenterY()),
                width_,
                height_};
    }

    Rectangle RemoveFromLeft(int16_t pxToRemove)
    {
        const auto result = this->WithWidth(min(pxToRemove, width_));
        x_ += min(pxToRemove, width_);
        width_ = max(int16_t(width_ - pxToRemove), 0);
        return result;
    }

    Rectangle RemoveFromRight(int16_t pxToRemove)
    {
        const auto canRemove = min(pxToRemove, width_);
        width_ -= canRemove;
        return {int16_t(x_ + width_), y_, canRemove, height_};
    }

    Rectangle RemoveFromTop(int16_t pxToRemove)
    {
        const auto result = this->WithHeight(min(pxToRemove, height_));
        y_ += min(pxToRemove, height_);
        height_ = max(height_ - pxToRemove, 0);
        return result;
    }

    Rectangle RemoveFromBottom(int16_t pxToRemove)
    {
        const auto canRemove = min(pxToRemove, height_);
        height_ -= canRemove;
        return {x_, int16_t(y_ + height_), width_, canRemove};
    }

    Rectangle AlignedWithin(const Rectangle& other, Alignment alignment) const
    {
        switch(alignment)
        {
            case Alignment::topLeft:
                return {other.x_, other.y_, width_, height_};
            case Alignment::topRight:
                return {int16_t(other.GetRight() - width_),
                        other.y_,
                        width_,
                        height_};
            case Alignment::topCentered:
                return {int16_t(x_ + (other.GetCenterX() - GetCenterX())),
                        other.y_,
                        width_,
                        height_};
            case Alignment::bottomLeft:
                return {other.x_,
                        int16_t(other.GetBottom() - height_),
                        width_,
                        height_};
            case Alignment::bottomRight:
                return {int16_t(other.GetRight() - width_),
                        int16_t(other.GetBottom() - height_),
                        width_,
                        height_};
            case Alignment::bottomCentered:
                return {int16_t(x_ + (other.GetCenterX() - GetCenterX())),
                        int16_t(other.GetBottom() - height_),
                        width_,
                        height_};
            case Alignment::centeredLeft:
                return {other.x_,
                        int16_t(y_ + (other.GetCenterY() - GetCenterY())),
                        width_,
                        height_};
            case Alignment::centeredRight:
                return {int16_t(other.GetRight() - width_),
                        int16_t(y_ + (other.GetCenterY() - GetCenterY())),
                        width_,
                        height_};
            case Alignment::centered:
                return {int16_t(x_ + (other.GetCenterX() - GetCenterX())),
                        int16_t(y_ + (other.GetCenterY() - GetCenterY())),
                        width_,
                        height_};
            default: return {};
        }
    }

  private:
    int16_t x_, y_, width_, height_;
    int16_t max(int16_t a, int16_t b) { return (a > b) ? a : b; }
    int16_t min(int16_t a, int16_t b) { return (a < b) ? a : b; }
};

} // namespace daisy