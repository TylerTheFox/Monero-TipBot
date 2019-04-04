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

#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/SocketStream.h"
#include "Poco/StreamCopier.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include <Poco/Net/HTTPResponse.h>

#include <fstream>

bool Util::doesWalletExist(const std::string & name)
{
    return Poco::File(name + WALLET_KEYS_EXTENSION).exists();
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

void Util::send_http_post(const std::string & hostname, const std::string & uri, unsigned short port, const std::string & body, const std::string & ContentType)
{
    // Networking
    Poco::Net::HTTPClientSession s(hostname, port);
    Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, uri);
    Poco::Net::HTTPResponse res;

    // Setup Request
    request.setContentType(ContentType);
    request.setContentLength(body.length());
    s.sendRequest(request) << body; // Send POST body
}

void Util::write_data_to_file(const std::string & filename, const std::string & data, bool truncate)
{
    std::ofstream out(filename, static_cast<std::ios_base::openmode>(truncate ? std::ios::trunc : std::ios::out));
    if (out.is_open())
    {
        out << data;
        out.close();
    }
}

std::string Util::read_data_from_file(const std::string & filename)
{
    std::ifstream in(filename);
    if (in.is_open())
        return { std::istreambuf_iterator<char>(in),std::istreambuf_iterator<char>() };
    return {};
}
