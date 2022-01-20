#ifndef IMAGE_H
#define IMAGE_H

class Image
{
private:
    void *data;
    size_t size;

public:
    int width, height;
    size_t step;

    // Constructors
    Image(void *data, size_t size, int width, int height, size_t step);
    Image();

    // Copy, Move, Destroy
    Image(const Image& other);
    Image(Image&& other) noexcept;
    ~Image();

    // Operators
    Image& operator=(Image other);

    // Swap
    friend void swap(Image& a, Image& b);

    // Methods
    void* get() const { return data; };
};


#endif // IMAGE_H
