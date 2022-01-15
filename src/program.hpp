#ifndef PROGRAM_H
#define PROGRAM_H

#include <string>
#include "image.hpp"
#include "input.hpp"

// Assuming a Windows program
class Program
{
private:
    std::string name;

public:
    Program(std::string name);
    ~Program();

    Image screenshot() const;
    bool input(Input in) const;

    void loop() const;
};



#endif // PROGRAM_H
