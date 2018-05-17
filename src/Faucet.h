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

#define MICROSECOND_HOUR            3600000000
#define MICROSECOND_DAY             (MICROSECOND_HOUR*24.0)

class Faucet : public AppBaseClass
{
public:
    Faucet();
    virtual ~Faucet() = default;

    void                                save();
    void                                load();
    void                                setAccount(Account *);
    iterator                            begin();
    const_iterator                      begin() const;
    const_iterator                      cbegin() const;

    iterator                            end();
    const_iterator                      end() const;
    const_iterator                      cend() const;

    void                                help(TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me) const;
    void                                take(TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me) const;
    void                                status(TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me) const;

private:
    std::vector<struct Command>     Commands;
};
