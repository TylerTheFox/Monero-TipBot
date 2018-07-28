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
#include "Util.h"
#include "RPC.h"
#include "Poco/File.h"
#include "Config.h"

bool Util::doesWalletExist(const std::string & name)
{
    return Poco::File(name).exists();
}

bool Util::doesWalletExist(DiscordID DIS_ID)
{
    return Util::doesWalletExist(Util::getWalletStrFromIID(DIS_ID));
}

std::string Util::getWalletStrFromIID(DiscordID DIS_ID)
{
    return Poco::format(DISCORD_WALLET_MASK, DIS_ID);
}

bool Util::parseQuotedString(const Poco::StringTokenizer & cmd, unsigned int start_idx, std::string & str, unsigned int & ret_idx)
{
    if (cmd[start_idx].size() > 2 && cmd[start_idx].at(0) == '\\' && cmd[start_idx].at(1) == '\"')
    {
        bool valid = false;
        for (ret_idx = start_idx; ret_idx < cmd.count(); ret_idx++)
        {
            str += Poco::replace(cmd[ret_idx], "\\\"", "") + " ";

            if (cmd[ret_idx].size() > 2 && cmd[ret_idx].at(cmd[ret_idx].size() - 2) == '\\' && cmd[ret_idx].at(cmd[ret_idx].size() - 1) == '\"')
            {
                ret_idx++;
                valid = true;
                break;
            }
        }

        if (!valid)
        {
            return false;
        }

        str.pop_back();
    }
    else return false;
    return true;
}