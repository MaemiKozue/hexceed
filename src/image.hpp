#ifndef IMAGE_H
#define IMAGE_H

class Image
{
private:
    void *data;
public:
    int width, height;
    size_t step;
    Image();
    Image(void *data, int width, int height, size_t step);
    ~Image();

    void* get() const { return data; };
};


#endif // IMAGE_H
