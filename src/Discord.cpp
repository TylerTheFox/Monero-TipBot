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
#include "Poco/Thread.h"
#include "RPCManager.h"
#include <fstream>
#include <memory>
#include <map>
#include <utility>

#include "cereal/archives/json.hpp"
#include "cereal/types/map.hpp"
#include "cereal/types/set.hpp"
#include "Tip.h"
#include "Faucet.h"
#include "RPCException.h"
#include "Poco/StringTokenizer.h"
#include "Lottery.h"
#include "Poco/ThreadTarget.h"
#include "Poco/Exception.h"
#include "CLI.h"

const char *aboutStr =
"```TipBot v%?i.%?i (Config: v%?i.%?i)\\n"
"(C) Brandan Tyler Lasley 2018\\n"
"Github: https://github.com/Brandantl/Monero-TipBot \\n"
"BTC: 1KsX66J98WMgtSbFA5UZhVDn1iuhN5B6Hm\\n"
"ITNS: iz5ZrkSjiYiCMMzPKY8JANbHuyChEHh8aEVHNCcRa2nFaSKPqKwGCGuUMUMNWRyTNKewpk9vHFTVsHu32X3P8QJD21mfWJogf\\n"
"XMR: 44DudyMoSZ5as1Q9MTV6ydh4BYT6BMCvxNZ8HAgeZo9SatDVixVjZzvRiq9fiTneykievrWjrUvsy2dKciwwoUv15B9MzWS\\n```";

TIPBOT::~TIPBOT()
{
    this->AppSave();

    GlobalConfig.General.Shutdown = true;
    while (GlobalConfig.General.Threads);
}

void TIPBOT::init()
{
    try
    {
        PLog = &Poco::Logger::get("Tipbot");

        Apps = {
            {(std::shared_ptr<AppBaseClass>(std::make_unique<CLI>(this)))},
            {(std::shared_ptr<AppBaseClass>(std::make_unique<Tip>()))},
            {(std::shared_ptr<AppBaseClass>(std::make_unique<Faucet>()))},
            {(std::shared_ptr<AppBaseClass>(std::make_unique<Lottery>(this)))}
        };

        for (auto & app : Apps)
            app->load();
    }
    catch (AppGeneralException & exp)
    {
        PLog->error("FATAL ERROR (APPERR): COULD NOT BOOT! RESON: %s --- %s", std::string(exp.what()), exp.getGeneralError());
        GlobalConfig.General.Shutdown = true;
        this->quit();
    }
    catch (Poco::Exception & exp)
    {
        PLog->error("FATAL ERROR (POCO): COULD NOT BOOT! RESON: %s", std::string(exp.what()));
        GlobalConfig.General.Shutdown = true;
        this->quit();
    }
    catch (cereal::Exception exp)
    {
        PLog->error("FATAL ERROR (CEREAL): COULD NOT BOOT! RESON: %s", std::string(exp.what()));
        GlobalConfig.General.Shutdown = true;
        this->quit();
    }
}

int TIPBOT::getDiscordChannelType(SleepyDiscord::Snowflake<SleepyDiscord::Channel> id)
{
    Poco::JSON::Parser          parser;
    Poco::JSON::Object::Ptr     object;
    std::string                 clientID;
    auto response = getChannel(id);
    object = parser.parse(response.text).extract<Poco::JSON::Object::Ptr>();
    return object->getValue<int>("type");
}

std::string TIPBOT::getDiscordDMChannel(DiscordID id)
{
    Poco::JSON::Parser      parser;
    Poco::JSON::Object::Ptr object;
    std::string             clientID;
    auto response = createDirectMessageChannel(Poco::format("%Lu", id));
    object = parser.parse(response.text).extract<Poco::JSON::Object::Ptr>();
    return object->getValue<std::string>("id");
}

DiscordUser UknownUser = { 0, FIND_USER_UNKNOWN_USER, 0 };
const DiscordUser & TIPBOT::findUser(const DiscordID & id)
{
    if (id > 0)
    {
        // Find user
        for (auto & server : UserList)
        {
            for (auto & user : server.second)
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

            struct DiscordUser newUser = {};
            newUser.username = object->getValue<std::string>("username");
            newUser.id = TIPBOT::convertSnowflakeToInt64(object->getValue<std::string>("id"));
            newUser.join_epoch_time = ((newUser.id >> 22) + 1420070400000) * 1000;
            auto ret = UserList.begin()->second.insert(newUser);
            saveUserList();
            return *ret.first;
        }
    }

    // No idea.
    return UknownUser;
}

bool TIPBOT::isUserAdmin(const UserMessage& message)
{
    auto myid = message.User.id;
    
    // Bot is an automatic admin.
    if (myid == RPCMan->getBotDiscordID()) return true;

    for (auto adminid : GlobalConfig.General.Admins)
    {
        if (myid == adminid)
            return true;
    }
    return false;
}


void dispatcher(const std::function<void(TIPBOT *, const UserMessage&, const Command &)> & func, TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me)
{
    static Poco::Logger & tlog = Poco::Logger::get("CommandDispatch");
    GlobalConfig.General.Threads++;

    if (!GlobalConfig.General.Shutdown)
    {
        try
        {
            func(DiscordPtr, message, me);
        }
        catch (const Poco::Exception & exp)
        {
            tlog.error("Poco Error: --- %s", std::string(exp.what()));
            DiscordPtr->sendMessage(message.Channel.id_str, "Poco Error: ---" + std::string(exp.what()) + " :cold_sweat:");
        }
        catch (const SleepyDiscord::ErrorCode & exp)
        {
            tlog.error("Sleepy Discord Error: --- %?i", exp);
        }
        catch (AppGeneralException & exp)
        {
            tlog.error("App Error: --- %s: %s", std::string(exp.what()), exp.getGeneralError());
            DiscordPtr->sendMessage(message.Channel.id_str, std::string(exp.what()) + " --- " + exp.getGeneralError() + " :cold_sweat:");
        }
    }

    GlobalConfig.General.Threads--;
}

void TIPBOT::ProcessCommand(const UserMessage & message)
{
    for (const auto & ptr : Apps)
    {
        for (const auto & command : *ptr.get())
        {
            try
            {
                Poco::StringTokenizer cmd(message.Message, " ");

                if (command.name == Poco::toLower(cmd[0]))
                {
                    if ((command.ChannelPermission == AllowChannelTypes::Any) || (message.ChannelPerm == command.ChannelPermission || message.ChannelPerm == AllowChannelTypes::CLI))
                    {
                        // Check if CLI is making the commands for the CLI command.
                        // If not continue.
                        if (command.ChannelPermission == AllowChannelTypes::CLI && message.ChannelPerm != AllowChannelTypes::CLI)
                            break;

                        if (command.opensWallet)
                            ptr->setAccount(&RPCMan->getAccount(message.User.id));
                        else  ptr->setAccount(nullptr);

                        if (TIPBOT::isCommandAllowedToBeExecuted(message, command))
                        {
                            PLog->information("User %s issued command: %s", message.User.id_str, message.Message);
                            // Create command thread
                            std::thread t1(dispatcher, command.func, this, message, command);
                            t1.detach();
                        }
                    }
                    break;
                }
            }
            catch (const Poco::Exception & exp)
            {
                PLog->error("Poco Error: --- %s", std::string(exp.what()));
                sendMessage(message.Channel.id_str, "Poco Error: ---" + std::string(exp.what()) + " :cold_sweat:");
            }
            catch (const SleepyDiscord::ErrorCode & exp)
            {
                PLog->error("Sleepy Discord Error: --- %?i", exp);
            }
            catch (AppGeneralException & exp)
            {
                PLog->error("App Error: --- %s: %s", std::string(exp.what()), exp.getGeneralError());
                sendMessage(message.Channel.id_str, std::string(exp.what()) + " --- " + exp.getGeneralError() + " :cold_sweat:");
            }
        }
    }
}

void TIPBOT::CommandParseError(const UserMessage& message, const Command& me)
{
    PLog->warning("User command error: User %s gave '%s' expected format '%s'", message.User.id_str, message.Message, me.params);
    sendMessage(message.Channel.id_str, Poco::format("Command Error --- Correct Usage: %s %s :cold_sweat:", me.name, me.params));
}

bool TIPBOT::isCommandAllowedToBeExecuted(const UserMessage& message, const Command& command)
{
    return !command.adminTools || (command.adminTools && (message.ChannelPerm == AllowChannelTypes::Private || message.ChannelPerm == AllowChannelTypes::CLI || command.ChannelPermission == AllowChannelTypes::Any) && TIPBOT::isUserAdmin(message));
}

std::string TIPBOT::generateHelpText(const std::string & title, const std::vector<Command>& cmds, const UserMessage& message)
{
    std::stringstream ss;
    ss << title;
    ss << "```";
    for (auto cmd : cmds)
    {
        if (TIPBOT::isCommandAllowedToBeExecuted(message, cmd))
        {
            // Check if CLI is making the commands for the CLI command.
            // If not continue.
            if (cmd.ChannelPermission == AllowChannelTypes::CLI && message.ChannelPerm != AllowChannelTypes::CLI)
                continue;

            ss << cmd.name << " " << cmd.params;
            if (cmd.ChannelPermission != AllowChannelTypes::Any && cmd.ChannelPermission != AllowChannelTypes::CLI)
            {
                ss << " -- " << AllowChannelTypeNames[static_cast<int>(cmd.ChannelPermission)];
            }
            if (cmd.adminTools)
            {
                ss << " -- ADMIN ONLY";
            }
            ss << "\\n";
        }
    }
    ss << "```";
    return ss.str();
}

void TIPBOT::onMessage(SleepyDiscord::Message old_message)
{
    if (!old_message.content.empty() && old_message.content.at(0) == '!')
        ProcessCommand(ConvertSleepyDiscordMsg(old_message));
}

#define DISCORD_MAX_GET_USERS 1000
void getDiscordUsers(TIPBOT & me, std::set<DiscordUser> & myList, const SleepyDiscord::Snowflake<SleepyDiscord::Server> & snowyServer, const unsigned short & limit, const SleepyDiscord::Snowflake<SleepyDiscord::User> & snowyUser)
{
    auto guildInfo = me.listMembers(snowyServer, limit, snowyUser).vector();

    struct DiscordUser newUser = {};
    for (auto user : guildInfo)
    {
        newUser.username = user.user.username;
        newUser.id = TIPBOT::convertSnowflakeToInt64(user.user.ID);
        newUser.join_epoch_time = ((newUser.id >> 22) + 1420070400000) * 1000;
        myList.insert(newUser);
    }

    if (guildInfo.size() == limit)
    {
        Poco::Thread::sleep(3000); // Wait a bit.
        getDiscordUsers(me, myList, snowyServer, limit, guildInfo[limit - 1].user.ID);
    }
}

void TIPBOT::onReady(SleepyDiscord::Ready readyData)
{
    // Start application
    init();

    loadUserList();
    BotUser = readyData.user;
    RPCMan->setBotUser(convertSnowflakeToInt64(BotUser.ID));
    refreshUserList();
}

void TIPBOT::refreshUserList()
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

void TIPBOT::saveUserList()
{
    std::ofstream out(DISCORD_USER_CACHE_FILENAME, std::ios::trunc);
    if (out.is_open())
    {
        PLog->information("Saving discord user list to disk...");
        {
            cereal::JSONOutputArchive ar(out);
            ar(CEREAL_NVP(UserList));
        }
        out.close();
    }
}

const struct TopTakerStruct TIPBOT::findTopTaker()
{
    TopTakerStruct me;
    std::map<DiscordID, std::uint64_t> topTakerList;
    for (const auto & mp : UserList)
    {
        for (const auto & usr : mp.second)
        {
            topTakerList[usr.id] += usr.total_faucet_itns_sent;
        }
    }

    auto TopTaker = std::max_element(topTakerList.begin(), topTakerList.end(),
        [](const std::pair<DiscordID, std::uint64_t>& p1, const std::pair<DiscordID, std::uint64_t>& p2) {
        return p1.second < p2.second; });

    const auto & TopDonorUser = findUser(TopTaker->first);

    me.me = TopDonorUser;
    me.amount = TopTaker->second;
    return me;
}

void TIPBOT::AppSave()
{
    for (auto & app : Apps)
        app->save();
}

const DiscordConversion Discord_CLI_Conversion[]
{
        { "\\n",    "\n" },
        { "```",    "\n" },
        { "``",     "\"" },
};


void TIPBOT::SendMsg(const UserMessage & data, std::string message)
{
    if (data.ChannelPerm == AllowChannelTypes::CLI)
    {
        // Replace Discord's custom formatting with CLI formatting.
        std::string out = message;

        for (auto elm : Discord_CLI_Conversion)
        {
            out = Poco::replace(out, elm.strold, elm.strnew);
        }

        PLog->information(out);
    }
    else
    {
        sendMessage(data.Channel.id_str, message);
    }
}

std::uint64_t TIPBOT::totalFaucetAmount()
{
    std::uint64_t amount = 0;
    for (auto & server : UserList)
    {
        for (auto & user : server.second)
        {
            amount += user.total_faucet_itns_sent;
        }
    }
    return amount;
}

void TIPBOT::loadUserList()
{
    std::ifstream in(DISCORD_USER_CACHE_FILENAME);
    if (in.is_open())
    {
        PLog->information("Loading discord user list from disk...");
        {
            cereal::JSONInputArchive ar(in);
            ar(CEREAL_NVP(UserList));
        }
        in.close();
    }
}

UserMessage TIPBOT::ConvertSleepyDiscordMsg(const SleepyDiscord::Message & message)
{
    UserMessage UsrMsg = {};

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
        UsrMsg.Mentions.emplace_back(m);
    }

    return UsrMsg;
}

uint8_t TIPBOT::getUserLang(const DiscordID & id)
{
    // Find user
    for (auto & server : UserList)
    {
        for (auto & user : server.second)
        {
            if (id == user.id) return user.language;
        }
    }

    return 0;
}

uint8_t TIPBOT::getUserLang(const SleepyDiscord::Snowflake<SleepyDiscord::User> & usr)
{
    return getUserLang(convertSnowflakeToInt64(usr));
}
