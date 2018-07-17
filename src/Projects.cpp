/*
Copyright(C) 2018 Brandan Tyler Lasley

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.
*/
#include "Projects.h"

#define CLASS_RESOLUTION(x) std::bind(&Projects::x, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
Projects::Projects(TIPBOT * DPTR) : enabled(true), PLog(nullptr), DiscordPtr(DPTR)
{
    Commands =
    {
        // Nothing yet.
    };
    PLog = &Poco::Logger::get("Projects");
}

void Projects::save()
{
}

void Projects::load()
{

}

void Projects::setAccount(Account*)
{
    // Do nothing, we construct this parameter since its pure virtual and we dont need it in this class.
}

iterator Projects::begin()
{
    return Commands.begin();
}

const_iterator Projects::begin() const
{
    return Commands.begin();
}

const_iterator Projects::cbegin() const
{
    return Commands.cbegin();
}

iterator Projects::end()
{
    return Commands.end();
}

const_iterator Projects::end() const
{
    return Commands.end();

}

const_iterator Projects::cend() const
{
    return Commands.cend();
}
