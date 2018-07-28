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

// This is just a dummy service used for memory testing.
#pragma once
#include "../Core/Tipbot.h"
#include <string>

class Dummy : public TIPBOT {
public:
    Dummy(const std::string &);
    ~Dummy();

    void                    start();
    void                    broadcastMsg(DiscordID channel, std::string message);
    void                    broadcastDirectMsg(DiscordID user, std::string message);
    const DiscordUser &     getUserFromServer(DiscordID user);
    void                    _shutdown();
};