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
#pragma once
#include "../Core/Tipbot.h"
#include "../Core/Account.h"
#include "../Core/AppBaseClass.h"
#include "Poco/Logger.h"

struct DiscordConversion
{
    std::string strold;
    std::string strnew;
};

class CLI : public AppBaseClass
{
public:
    CLI(TIPBOT * DPTR);
    virtual ~CLI();

    void                                cli_main();

    UserMessage                         generateUsrMsg(std::string msg);
};