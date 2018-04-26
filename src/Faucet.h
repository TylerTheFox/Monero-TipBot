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
#include "Discord.h"
#include "Account.h"
#include "AppBaseClass.h"

#define FAUCET_PERCENTAGE_ALLOWANCE 0.01
#define MIN_DISCORD_ACCOUNT_IN_DAYS 7
#define FAUCET_TIMEOUT              6

class Faucet : public AppBaseClass
{
public:
    Faucet();
    virtual ~Faucet() = default;

    void            setAccount(Account *);
    iterator        begin();
    const_iterator  begin() const;
    const_iterator  cbegin() const;

    iterator        end();
    const_iterator  end() const;
    const_iterator  cend() const;

    void                                help(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me);
    void                                take(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me);
    void                                status(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me);

private:
    std::vector<struct Command>     Commands;
};
