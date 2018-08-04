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
#include "../Core/AppBaseClass.h"
#include "../Core/Script.h"

class ScriptCLI : public AppBaseClass
{
public:
    ScriptCLI(TIPBOT * DPTR, Script * scrptr);
    virtual ~ScriptCLI() = default;

    void script_help(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void script_start(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void script_shutdown(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void script_restart(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void script_count(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void script_list(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void script_shutdown_all(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
private:
    Script * _scrptr;
};