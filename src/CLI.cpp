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
#include "CLI.h"
#include "Config.h"
#include "Poco/Thread.h"
#include "RPCManager.h"
#include <Poco/StringTokenizer.h>
#include <thread>

CLI::CLI(TIPBOT * dptr)
{
    PLog = &Poco::Logger::get("CLI");

    DiscordPtr = dptr;

    // Create CLI thread
    std::thread t1(&CLI::cli_main, this);
    t1.detach();
}

CLI::~CLI()
{

}

void CLI::cli_main()
{
    GlobalConfig.General.Threads++;

    static std::string buffer;
    static bool input_thread_active = false;
    static Poco::Mutex mu;

    if (!input_thread_active)
    {
        // Create CLI thread
        std::thread t1([&]()
        {
            while (true)
            {
                mu.lock();
                std::getline(std::cin, buffer);
                mu.unlock();
                Poco::Thread::sleep(10);
            }
        });
        t1.detach();
        input_thread_active = true;
    }

    std::string loc_buff;
    while (!GlobalConfig.General.Shutdown)
    {
        if (mu.tryLock())
        {
            loc_buff = buffer;
            buffer.clear();
            mu.unlock();
            if (!loc_buff.empty())
            {
                PLog->information(loc_buff);
                DiscordPtr->ProcessCommand(generateUsrMsg(loc_buff));
            }
        }
        Poco::Thread::sleep(1);
    }

    GlobalConfig.General.Threads--;
}

UserMessage CLI::generateUsrMsg(std::string msg)
{
    UserMessage ret = {};

    ret.Message = msg;
    ret.ChannelPerm = AllowChannelTypes::CLI;
    ret.User.id = RPCMan->getBotDiscordID();
    ret.User.id_str = Poco::format("%?i", ret.User.id);
    ret.User.username = "CLI";
    ret.User.discriminator = "CLI";

    // Generate Mentions (By ID)
    try
    {
        Poco::StringTokenizer cmd(ret.Message, " ");
        std::string usr;
        Snowflake newUsr;
        for (auto elm : cmd)
        {
            if (!elm.empty() && elm.at(0) == '@')
            {
                usr = Poco::replace(elm, "@", "");
                const auto usrid = Poco::NumberParser::parseUnsigned64(usr);
                newUsr.id = usrid;
                newUsr.id_str = usr;
                auto disUsr = DiscordPtr->findUser(newUsr.id);
                if (disUsr.username != FIND_USER_UNKNOWN_USER)
                {
                    newUsr.discriminator = "????";
                    newUsr.username = disUsr.username;
                    ret.Mentions.emplace_back(newUsr);
                }
            }
        }
    }
    catch (Poco::Exception exp)
    {
        // We don't care, intentionally left blank.
    }

    return ret;
}

void CLI::save()
{
}

void CLI::load()
{
}

void CLI::setAccount(Account* acc)
{

}

iterator CLI::begin()
{
    return Commands.begin();
}

const_iterator CLI::begin() const
{
    return Commands.begin();
}

const_iterator CLI::cbegin() const
{
    return Commands.cbegin();
}

iterator CLI::end()
{
    return Commands.end();
}

const_iterator CLI::end() const
{
    return Commands.end();
}

const_iterator CLI::cend() const
{
    return Commands.cend();
}