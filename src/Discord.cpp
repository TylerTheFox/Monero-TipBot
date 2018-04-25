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
#include "Discord.h"
#include "Poco/NumberParser.h"
#include "DiscordCommands.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Thread.h"
#include "RPCManager.h"
#include <fstream>

#include "cereal/archives/json.hpp"
#include "cereal/types/map.hpp"
#include "cereal/types/set.hpp"

int ITNS_TIPBOT::getDiscordChannelType(SleepyDiscord::Snowflake<SleepyDiscord::Channel> id)
{
    Poco::JSON::Parser          parser;
    Poco::JSON::Object::Ptr     object;
    std::string                 clientID;
    auto response = getChannel(id);
    object = parser.parse(response.text).extract<Poco::JSON::Object::Ptr>();
    return object->getValue<int>("type");
}

std::string ITNS_TIPBOT::getDiscordDMChannel(DiscordID id)
{
    Poco::JSON::Parser      parser;
    Poco::JSON::Object::Ptr object;
    std::string             clientID;
    auto response = createDirectMessageChannel(Poco::format("%Lu", id));
    object = parser.parse(response.text).extract<Poco::JSON::Object::Ptr>();
    return object->getValue<std::string>("id");
}

const DiscordUser UknownUser = { 0, "Unknown User", 0 };
const DiscordUser & ITNS_TIPBOT::findUser(const DiscordID & id)
{
    if (id > 0)
    {
        // Find user
        for (const auto & server : UserList)
        {
            for (const auto & user : server.second)
            {
                if (id == user.id) return user;
            }
        }

        // User not found... Try and pull from Discord API
        auto response = getUser(Poco::format("%?i", id));
        if (response.statusCode == 200)
        {
            Poco::JSON::Parser      parser;
            Poco::JSON::Object::Ptr object;
            object = parser.parse(response.text).extract<Poco::JSON::Object::Ptr>();

            struct DiscordUser newUser;
            newUser.username = object->getValue<std::string>("username");
            newUser.id = ITNS_TIPBOT::convertSnowflakeToInt64(object->getValue<std::string>("id"));
            newUser.join_epoch_time = ((newUser.id >> 22) + 1420070400000) / 1000;
            auto ret = UserList.begin()->second.insert(newUser);
            saveUserList();
            return *ret.first;
        }
    }

    // No idea.
    return UknownUser;
}

void ITNS_TIPBOT::onMessage(SleepyDiscord::Message message)
{
    DiscordCommands::ProcessCommand(this, message);
}

#define DISCORD_MAX_GET_USERS 2
void getDiscordUsers(ITNS_TIPBOT & me, std::set<DiscordUser> & myList, const SleepyDiscord::Snowflake<SleepyDiscord::Server> & snowyServer, const unsigned short & limit, const SleepyDiscord::Snowflake<SleepyDiscord::User> & snowyUser)
{
    auto guildInfo = me.listMembers(snowyServer, limit, snowyUser).vector();

    for (auto user : guildInfo)
    {
        struct DiscordUser newUser;
        newUser.username = user.user.username;
        newUser.id = ITNS_TIPBOT::convertSnowflakeToInt64(user.user.ID);
        newUser.join_epoch_time = ((newUser.id >> 22) + 1420070400000) / 1000;
        myList.insert(newUser);
    }

    if (guildInfo.size() == limit)
    {
        Poco::Thread::sleep(3000); // Wait a bit.
        getDiscordUsers(me, myList, snowyServer, limit, guildInfo[limit - 1].user.ID);
    }
}

void ITNS_TIPBOT::onReady(SleepyDiscord::Ready readyData)
{
    loadUserList();
    BotUser = readyData.user;
    RPCMan.setBotUser(convertSnowflakeToInt64(BotUser.ID));
    refreshUserList();
}

void ITNS_TIPBOT::refreshUserList()
{
    auto servs = this->getServers().vector();

    std::cout << "Loading Discord Users...\n";
    for (auto serv : servs)
    {
        Poco::Thread::sleep(3000); // Wait a bit.
        if (UserList.empty())
        {
            getDiscordUsers(*this, UserList[convertSnowflakeToInt64(serv.ID)], serv.ID, DISCORD_MAX_GET_USERS, "");
        }
        else
        {
            getDiscordUsers(*this, UserList[convertSnowflakeToInt64(serv.ID)], serv.ID, DISCORD_MAX_GET_USERS, Poco::format("%?i", UserList[convertSnowflakeToInt64(serv.ID)].rbegin()->id));
        }
        std::cout << "Saving Discord Users To Disk...\n";
        saveUserList();
    }
    std::cout << "Discord Users Load Completed... Ready!\n";
}

void ITNS_TIPBOT::saveUserList()
{
    std::ofstream out(DISCORD_USER_CACHE_FILENAME, std::ios::trunc);
    if (out.is_open())
    {
        std::cout << "Saving wallet data to disk...\n";
        {
            cereal::JSONOutputArchive ar(out);
            ar(CEREAL_NVP(UserList));
        }
        out.close();
    }
}

void ITNS_TIPBOT::loadUserList()
{
    std::ifstream in(DISCORD_USER_CACHE_FILENAME);
    if (in.is_open())
    {
        std::cout << "Saving wallet data to disk...\n";
        {
            cereal::JSONInputArchive ar(in);
            ar(CEREAL_NVP(UserList));
        }
        in.close();
    }
}
