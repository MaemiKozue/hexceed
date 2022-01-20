#include "image.hpp"

#include <iostream>
#include <cstdlib>

Image::Image(void *data, int width, int height, size_t step) :
    data(data),
    width(width),
    height(height),
    step(step)
{
}

Image::Image() :
    Image(NULL, 0, 0, 0)
{
}

Image::~Image()
{
    std::cout << "Freed" << std::endl;
    free(this->data);
}
