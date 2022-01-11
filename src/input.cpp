#include "input.hpp"

#include <iostream>

std::ostream& operator<<(std::ostream& out, Input in) 
{
    switch (in.getType())
    {
    case MOUSE:
        out << "MousePress(" << in.getButton() << " at " << in.getX() << ", " << in.getY() << ")";
        break;
    
    case KEYBOARD:
        out << "KeyPress(" << in.getKey() << ")";
        break;

    default:
        out << "UNKOWN()";
        break;
    }
    return out;
}

Input::Input(MOUSE_BUTTON mb, int x, int y) :
    type(MOUSE),
    mb(mb),
    x(x),
    y(y)
{
}

Input::Input(KEY key) :
    type(KEYBOARD),
    key(key)
{
}


Input::~Input()
{
}

InputType Input::getType() const 
{
    return this->type;
}

int Input::getX() const 
{
    return this->x;
}

int Input::getY() const 
{
    return this->y;
}

MOUSE_BUTTON Input::getButton() const 
{
    return this->mb;
}

KEY Input::getKey() const 
{
    return this->key;
}
