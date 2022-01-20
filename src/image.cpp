#include "image.hpp"

#include <iostream>
#include <cstdlib>


Image::Image(void *data, size_t size, int width, int height, size_t step) :
    data(data),
    size(size),
    width(width),
    height(height),
    step(step)
{
}


Image::Image() :
    Image(NULL, 0, 0, 0, 0)
{
}


Image::Image(const Image& other) :
    Image(NULL, other.size, other.width, other.height, other.step)
{
    this->data = malloc(this->size);
    memcpy(this->data, other.data, this->size);
}


Image::Image(Image&& other) noexcept :
    Image()
{
    swap(*this, other);
}


Image& Image::operator=(Image other)
{
    swap(*this, other);
    return *this;
}


void swap (Image& a, Image& b)
{
    using std::swap;

    swap(a.data, b.data);
    swap(a.size, b.size);
    swap(a.width, b.width);
    swap(a.height, b.height);
    swap(a.step, b.step);
}


Image::~Image()
{
    free(this->data);
}
