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
#include "Script.h"
#include <string>
#include <memory>
class Account;

class AppBaseClass
{
public:
    AppBaseClass(TIPBOT * DPTR) :AppName("Unknown App"), HelpCommand(nullptr), enabled(false), DiscordPtr(DPTR)
    {
#ifndef NO_CHAISCRIPT
        m.reset(new chaiscript::Module);
#endif
    }

    virtual                             ~AppBaseClass() = default;

    virtual void                        setAccount(Account *) {}
    virtual void                        save() {};
    virtual void                        load() {};

    iterator                            begin() { return Commands.begin(); };
    const_iterator                      begin() const { return Commands.begin(); };
    const_iterator                      cbegin() const { return Commands.cbegin(); };

    iterator                            end() { return Commands.end(); };
    const_iterator                      end() const { return Commands.end(); };
    const_iterator                      cend() const { return Commands.cend(); };

    virtual void                        run(const UserMessage & message) { };

    void                                setName(const std::string & name) { AppName = name, PLog = &Poco::Logger::get(AppName); }
    const std::string &                 getName() const { return AppName; }
    void                                setHelpCommand(const Command & cmd) { HelpCommand = &cmd; }
    const Command *                     getHelpCommand()  const { return HelpCommand; }
    bool                                isEnabled() const { return enabled; }
#ifndef NO_CHAISCRIPT
    const chaiscript::ModulePtr &       getScriptModule() const { return m; }
#endif
protected:
    TIPBOT * DiscordPtr;
#ifndef NO_CHAISCRIPT
    chaiscript::ModulePtr               m;
#endif
    std::string                         AppName;
    bool                                enabled;
    const struct Command *              HelpCommand;
    std::vector<struct Command>         Commands;
    Poco::Logger*                       PLog;
};
