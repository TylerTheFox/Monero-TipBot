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
    CLI(TIPBOT * dptr);
    virtual ~CLI();

    void                                save();
    void                                load();
    void                                setAccount(Account *);
    iterator                            begin();
    const_iterator                      begin() const;
    const_iterator                      cbegin() const;

    iterator                            end();
    const_iterator                      end() const;
    const_iterator                      cend() const;

    void                                cli_main();

    UserMessage                         generateUsrMsg(std::string msg);
private:
    TIPBOT*                             DiscordPtr;
    Poco::Logger*                       PLog;
    std::vector<struct Command>         Commands;
};