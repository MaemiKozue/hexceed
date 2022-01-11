#ifndef IMAGE_H
#define IMAGE_H

class Image
{
private:
    void *data;
public:
    Image(void *data);
    ~Image();
};


#endif // IMAGE_H
