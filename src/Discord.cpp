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
#include "Poco/JSON/Parser.h"
#include "Poco/Format.h"
#include "Poco/Thread.h"
#include "RPCManager.h"

template<typename t>
DiscordID convertSnowflakeToInt64(t id)
{
    return Poco::NumberParser::parseUnsigned64(static_cast<std::string>(id));
}

int Discord::getDiscordChannelType(SleepyDiscord::Snowflake<SleepyDiscord::Channel> id)
{
    try
    {
        Poco::JSON::Parser          parser;
        Poco::JSON::Object::Ptr     object;
        std::string                 clientID;
        auto response = getChannel(id);
        object = parser.parse(response.text).extract<Poco::JSON::Object::Ptr>();
        return object->getValue<int>("type");
    }
    catch (const websocketpp::exception & err)
    {
        PLog->error("websocketpp Code: %?i Message: %s", err.code(), err.m_msg);
    }
    catch (const SleepyDiscord::ErrorCode & exp)
    {
        PLog->error("Discord Error: %?i", exp);
    }
    return 0;
}

void Discord::start()
{
    PLog = &Poco::Logger::get("Discord");
    try
    {
        this->run();
    }
    catch (const websocketpp::exception & err)
    {
        GlobalConfig.General.Shutdown = true;
        PLog->error("websocketpp Code: %?i Message: %s", err.code(), err.m_msg);
    }
    catch (const SleepyDiscord::ErrorCode & exp)
    {
        GlobalConfig.General.Shutdown = true;
        PLog->error("Discord Error: %?i", exp);
    }
}

void  Discord::onMessage(SleepyDiscord::Message message)
{
    try
    {
        if (!message.content.empty() && message.content.at(0) == '!')
            ProcessCommand(ConvertSleepyDiscordMsg(message));
    }
    catch (const websocketpp::exception & err)
    {
        PLog->error("onMessage --- websocketpp Code: %?i Message: %s", err.code(), err.m_msg);
    }
    catch (const SleepyDiscord::ErrorCode & exp)
    {
        PLog->error("onMessage --- Discord Error: %?i", exp);
    }
}

void  Discord::onReady(SleepyDiscord::Ready readyData)
{
    try
    {
        // Start application
        tipbot_init();

        loadUserList();
        RPCMan->setBotUser(convertSnowflakeToInt64(readyData.user.ID));
        refreshUserList();
    }
    catch (const websocketpp::exception & err)
    {
        PLog->error("onReady --- websocketpp Code: %?i Message: %s", err.code(), err.m_msg);
    }
    catch (const SleepyDiscord::ErrorCode & exp)
    {
        PLog->error("onReady --- Discord Error: %?i", exp);
    }
}

void Discord::broadcastMsg(DiscordID channel, std::string message)
{
    try
    {
        sendMessage(Poco::format("%Lu", channel), message);
    }
    catch (const websocketpp::exception & err)
    {
        PLog->error("broadcastMsg --- websocketpp Code: %?i Message: %s", err.code(), err.m_msg);
    }
    catch (const SleepyDiscord::ErrorCode & exp)
    {
        PLog->error("broadcastMsg --- Discord Error: %?i", exp);
    }
}

void Discord::broadcastDirectMsg(DiscordID user, std::string message)
{
    try
    {
        Poco::JSON::Parser      parser;
        Poco::JSON::Object::Ptr object;
        std::string             clientID;
        auto response = createDirectMessageChannel(Poco::format("%Lu", user));
        object = parser.parse(response.text).extract<Poco::JSON::Object::Ptr>();
        sendMessage(object->getValue<std::string>("id"), message);
    }
    catch (const websocketpp::exception & err)
    {
        PLog->error("broadcastDirectMsg --- websocketpp Code: %?i Message: %s", err.code(), err.m_msg);
    }
    catch (const SleepyDiscord::ErrorCode & exp)
    {
        PLog->error("broadcastDirectMsg --- Discord Error: %?i", exp);
    }
}

UserMessage Discord::ConvertSleepyDiscordMsg(const SleepyDiscord::Message & message)
{
    UserMessage UsrMsg = {};

    try
    {
        UsrMsg.User.id_str = message.author.ID;
        UsrMsg.User.id = convertSnowflakeToInt64(UsrMsg.User.id_str);
        UsrMsg.User.username = message.author.username;
        UsrMsg.User.discriminator = message.author.discriminator;
        UsrMsg.Channel.id_str = message.channelID;
        UsrMsg.Channel.id = convertSnowflakeToInt64(UsrMsg.Channel.id_str);
        UsrMsg.ChannelPerm = static_cast<AllowChannelTypes>(getDiscordChannelType(UsrMsg.Channel.id_str));
        UsrMsg.Message = message.content;

        Snowflake m;
        for (auto men : message.mentions)
        {
            m.id_str = men.ID;
            m.id = convertSnowflakeToInt64(men.ID);
            m.username = men.username;
            m.discriminator = men.discriminator;
            UsrMsg.Mentions.emplace_back(m);
        }
    }
    catch (const websocketpp::exception & err)
    {
        PLog->error("ConvertSleepyDiscordMsg --- websocketpp Code: %?i Message: %s", err.code(), err.m_msg);
    }
    catch (const SleepyDiscord::ErrorCode & exp)
    {
        PLog->error("ConvertSleepyDiscordMsg --- Discord Error: %?i", exp);
    }

    return UsrMsg;
}

DiscordUser Discord::getUserFromServer(DiscordID user)
{
    struct DiscordUser newUser = {};

    try
    {
        auto response = getUser(Poco::format("%?i", user));
        if (response.statusCode == 200)
        {
            Poco::JSON::Parser      parser;
            Poco::JSON::Object::Ptr object;
            object = parser.parse(response.text).extract<Poco::JSON::Object::Ptr>();

            newUser.username = object->getValue<std::string>("username");
            newUser.id = convertSnowflakeToInt64(object->getValue<std::string>("id"));
            newUser.join_epoch_time = ((newUser.id >> 22) + 1420070400000) * 1000;
            auto ret = UserList.begin()->second.insert(newUser);
            saveUserList();
            return *ret.first;
        }

        newUser.username = FIND_USER_UNKNOWN_USER;
    }
    catch (const websocketpp::exception & err)
    {
        PLog->error("getUserFromServer --- websocketpp Code: %?i Message: %s", err.code(), err.m_msg);
    }
    catch (const SleepyDiscord::ErrorCode & exp)
    {
        PLog->error("getUserFromServer --- Discord Error: %?i", exp);
    }

    return newUser;
}

#define DISCORD_MAX_GET_USERS 1000
void getDiscordUsers(Discord & me, std::set<DiscordUser> & myList, const SleepyDiscord::Snowflake<SleepyDiscord::Server> & snowyServer, const unsigned short & limit, const SleepyDiscord::Snowflake<SleepyDiscord::User> & snowyUser)
{
    try
    {
        auto guildInfo = me.listMembers(snowyServer, limit, snowyUser).vector();

        struct DiscordUser newUser = {};
        for (auto user : guildInfo)
        {
            newUser.username = user.user.username;
            newUser.id = convertSnowflakeToInt64(user.user.ID);
            newUser.join_epoch_time = ((newUser.id >> 22) + 1420070400000) * 1000;
            myList.insert(newUser);
        }

        if (guildInfo.size() == limit)
        {
            Poco::Thread::sleep(3000); // Wait a bit.
            getDiscordUsers(me, myList, snowyServer, limit, guildInfo[limit - 1].user.ID);
        }
    }
    catch (const websocketpp::exception & err)
    {
        me.PLog->error("getDiscordUsers --- websocketpp Code: %?i Message: %s", err.code(), err.m_msg);
    }
    catch (const SleepyDiscord::ErrorCode & exp)
    {
        me.PLog->error("getDiscordUsers --- Discord Error: %?i", exp);
    }
}

void Discord::refreshUserList()
{
    try
    {
        auto servs = this->getServers().vector();

        PLog->information("Loading Discord Users...");
        for (auto serv : servs)
        {
            Poco::Thread::sleep(3000); // Wait a bit.
            if (UserList[convertSnowflakeToInt64(serv.ID)].empty())
            {
                getDiscordUsers(*this, UserList[convertSnowflakeToInt64(serv.ID)], serv.ID, DISCORD_MAX_GET_USERS, "");
            }
            else
            {
                getDiscordUsers(*this, UserList[convertSnowflakeToInt64(serv.ID)], serv.ID, DISCORD_MAX_GET_USERS, Poco::format("%?i", UserList[convertSnowflakeToInt64(serv.ID)].rbegin()->id));
            }
            PLog->information("Saving Discord Users To Disk...");
            saveUserList();
        }
        PLog->information("Discord Users Load Completed... Ready!");
    }
    catch (const websocketpp::exception & err)
    {
        PLog->error("refreshUserList --- websocketpp Code: %?i Message: %s", err.code(), err.m_msg);
    }
    catch (const SleepyDiscord::ErrorCode & exp)
    {
        PLog->error("refreshUserList --- Discord Error: %?i", exp);
    }
}

void Discord::shutdown()
{
    try
    {
        this->quit();
    }
    catch (const websocketpp::exception & err)
    {
        PLog->error("shutdown --- websocketpp Code: %?i Message: %s", err.code(), err.m_msg);
    }
    catch (const SleepyDiscord::ErrorCode & exp)
    {
        PLog->error("shutdown --- Discord Error: %?i", exp);
    }
}