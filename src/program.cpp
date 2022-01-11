#include "program.hpp"

#include <string>
#include <iostream>

Program::Program(std::string name) :
    name(name)
{
}

Program::~Program()
{
}


Image Program::screenshot() const
{
    std::cout << "Taking a screenshot from " << this->name << std::endl;
    return NULL;
}

bool Program::input(Input in) const
{
    std::cout << "Sending " << in << " to " << this->name << std::endl;
    return false;
}