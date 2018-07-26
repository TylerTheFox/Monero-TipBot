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
#include <string>
#include "Poco/Process.h"
#include "types.h"
#include "Poco/StringTokenizer.h"
#define MICROSECOND_HOUR            3600000000
#define MICROSECOND_DAY             (MICROSECOND_HOUR*24.0)
#define DISCORD_WALLET_MASK         "Discord-User-%Lu"

class Util
{
public:
    Util() = delete;
    ~Util() = delete;

    static bool doesWalletExist(const std::string & name);
    static bool doesWalletExist(DiscordID DIS_ID);

    static std::string getWalletStrFromIID(DiscordID DIS_ID);
    static bool parseQuotedString(const Poco::StringTokenizer & cmd, unsigned int start_idx, std::string & str, unsigned int & ret_idx);
};