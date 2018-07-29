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
#include "Tipbot.h"
#include <string>
class Account;

class AppBaseClass
{
public:
    AppBaseClass(TIPBOT * DPTR) :AppName("Unknown App"), HelpCommand(nullptr), enabled(false), DiscordPtr(DPTR) {};

    virtual                             ~AppBaseClass() = default;

    virtual void                        setAccount(Account *) {}
    virtual void                        save() {};
    virtual void                        load() {};

    iterator                            begin() { return Commands.begin(); };
    const_iterator                      begin() const { return Commands.begin();  };
    const_iterator                      cbegin() const { return Commands.cbegin();  };

    iterator                            end() { return Commands.end();  };
    const_iterator                      end() const { return Commands.end();  };
    const_iterator                      cend() const { return Commands.cend(); };

    virtual void                        run(const UserMessage & message) { };

    void                                setName(const std::string & name) { AppName = name, PLog = &Poco::Logger::get(AppName); }
    const std::string &                 getName() { return AppName; }
    void                                setHelpCommand(const Command & cmd) { HelpCommand = &cmd; }
    const Command *                     getHelpCommand() { return HelpCommand; }
    bool                                isEnabled() { return enabled; }
protected:
    TIPBOT *                            DiscordPtr;
    std::string                         AppName;
    bool                                enabled;
    const struct Command *              HelpCommand;
    std::vector<struct Command>         Commands;
    Poco::Logger*                       PLog;
};
