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
    AppBaseClass() : AppName("Unknown App"), HelpCommand(nullptr), enabled(false) {};
    virtual ~AppBaseClass() = default;

    virtual void            setAccount(Account *) = 0;
    virtual void            save() = 0;
    virtual void            load() = 0;

    virtual iterator        begin() = 0;
    virtual const_iterator  begin() const = 0;
    virtual const_iterator  cbegin() const = 0;

    virtual iterator        end() = 0;
    virtual const_iterator  end() const = 0;
    virtual const_iterator  cend() const = 0;

    virtual void            run(const UserMessage & message) { };
    
    void                    setName(const std::string & name) { AppName = name; }
    const std::string &     getName() { return AppName; }
    void                    setHelpCommand(const Command & cmd) { HelpCommand = &cmd; }
    const Command *         getHelpCommand() { return HelpCommand; }
    bool                    isEnabled() { return enabled; }
protected:
    std::string             AppName;
    bool                    enabled;
    const struct Command *  HelpCommand;
};
