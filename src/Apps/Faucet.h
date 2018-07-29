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
#include "../Core/Util.h"
#include "../Core/AppBaseClass.h"
#include "Poco/Logger.h"
#include "Poco/AutoPtr.h"


#define    FAUCET_SAVE_FILE       "FAUCET.JSON"
class Faucet : public AppBaseClass
{
public:
    Faucet(TIPBOT * DPTR);
    virtual ~Faucet() = default;

    void                                save();
    void                                load();

    void                                help(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me) const;
    void                                take(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void                                status(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me) const;
    void                                ToggleFaucet(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void                                award(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
};
