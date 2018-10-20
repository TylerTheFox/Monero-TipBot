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
#include "Tipbot.h"
#include "Poco/Thread.h"
#include "RPCManager.h"
#include <fstream>
#include <memory>
#include <map>
#include <utility>

#include "cereal/archives/json.hpp"
#include "cereal/types/map.hpp"
#include "cereal/types/set.hpp"
#include "cereal/types/unordered_map.hpp"

#include "RPCException.h"
#include "Poco/StringTokenizer.h"
#include "Poco/ThreadTarget.h"
#include "Poco/Exception.h"
#include <thread>

#include "../Apps/Tip.h"
#include "../Apps/CLI.h"
#include "../Apps/ChatRewards.h"
#include "../Apps/Projects.h"
#include "../Apps/Lottery.h"
#include "../Apps/Faucet.h"
#ifndef NO_CHAISCRIPT
#include "../Apps/ScriptCLI.h"
#endif

const char *aboutStr =
"```TipBot v%?i.%?i (Config: v%?i.%?i)\\n"
"(C) Brandan Tyler Lasley 2018\\n"
"Github: https://github.com/Brandantl/Monero-TipBot \\n"
"BTC: 1KsX66J98WMgtSbFA5UZhVDn1iuhN5B6Hm\\n"
"LTHN: iz5ZrkSjiYiCMMzPKY8JANbHuyChEHh8aEVHNCcRa2nFaSKPqKwGCGuUMUMNWRyTNKewpk9vHFTVsHu32X3P8QJD21mfWJogf\\n"
"XMR: 44DudyMoSZ5as1Q9MTV6ydh4BYT6BMCvxNZ8HAgeZo9SatDVixVjZzvRiq9fiTneykievrWjrUvsy2dKciwwoUv15B9MzWS\\n"
"MSR: 5h9GZz5bbvUK5TPb1KB8J7FnbQHyEd1z93scwhu3WZ9m3YJwCAUVyz3FoKh4JiTTWPKcGmJkxBWS2YkmzJoXTimqTbCKFKm\\n```";

bool TIPBOT::init = false;

TIPBOT::TIPBOT() : PLog(nullptr), ScriptEng(this)
{

}

TIPBOT::~TIPBOT()
{
    GlobalConfig.General.Shutdown = true;
#ifndef NO_CHAISCRIPT
    ScriptEng.clearAll();
#endif
    while (GlobalConfig.General.Threads) { Poco::Thread::sleep(1); }
    init = false;
}

void TIPBOT::shutdown()
{
    this->AppSave();

    PLog->information("Shutting Threads Down...");

#ifndef NO_CHAISCRIPT
    ScriptEng.clearAll();
#endif

    GlobalConfig.General.Shutdown = true;
    while (GlobalConfig.General.Threads > 1) { Poco::Thread::sleep(1); }

    PLog->information("All Threads Shutdown!");

    this->_shutdown();

    init = false;
}

void TIPBOT::tipbot_init()
{
    try
    {
        if (!init)
        {
            PLog = &Poco::Logger::get("Tipbot");

            Apps = {
                { (std::shared_ptr<AppBaseClass>(std::make_unique<CLI>(this))) },
                { (std::shared_ptr<AppBaseClass>(std::make_unique<Tip>(this))) },
                { (std::shared_ptr<AppBaseClass>(std::make_unique<Faucet>(this))) },
                { (std::shared_ptr<AppBaseClass>(std::make_unique<ChatRewards>(this))) },
                { (std::shared_ptr<AppBaseClass>(std::make_unique<Projects>(this))) },
                { (std::shared_ptr<AppBaseClass>(std::make_unique<Lottery>(this))) },
#ifndef NO_CHAISCRIPT
                { (std::shared_ptr<AppBaseClass>(std::make_unique<ScriptCLI>(this, &ScriptEng))) },
#endif
            };
            for (auto & app : Apps)
                app->load();

            loadUserList();
            LoadStats();

            // Upgrade save file
            if (VERSION_MAJOR != GlobalConfig.About.major || VERSION_MINOR != GlobalConfig.About.minor)
            {
                PLog->information("Upgrading Save file...");
                GlobalConfig.About.major = VERSION_MAJOR;
                GlobalConfig.About.minor = VERSION_MINOR;
                GlobalConfig.save_config();
                AppSave();
                SaveStats();
            }
#ifndef NO_CHAISCRIPT
            ScriptEng._load();
#endif
            init = true;
        }
    }
    catch (AppGeneralException & exp)
    {
        PLog->error("FATAL ERROR (APPERR): COULD NOT BOOT! REASON: %s --- %s", std::string(exp.what()), exp.getGeneralError());
        GlobalConfig.General.Shutdown = true;
        this->shutdown();
    }
    catch (Poco::Exception & exp)
    {
        PLog->error("FATAL ERROR (POCO): COULD NOT BOOT! REASON: %s", std::string(exp.what()));
        GlobalConfig.General.Shutdown = true;
        this->shutdown();
    }
    catch (cereal::Exception exp)
    {
        PLog->error("FATAL ERROR (CEREAL): COULD NOT BOOT! REASON: %s", std::string(exp.what()));
        GlobalConfig.General.Shutdown = true;
        this->shutdown();
    }
}

const PerformanceMap & TIPBOT::getPerformanceStats()
{
    return AppPerformanceStats;
}

const ExecuteCommandType & TIPBOT::getRunningCommands()
{
    return runningCommands;
}

DiscordUser UknownUser = { 0, FIND_USER_UNKNOWN_USER, 0 };
const DiscordUser & TIPBOT::findUser(const DiscordID & id)
{
    try
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

            return getUserFromServer(id);
        }
    }
    catch (...)
    {
        PLog->error("Error finding user!");
    }

    // No idea.
    return UknownUser;
}

bool TIPBOT::isUserAdmin(const UserMessage& message)
{
    const auto myid = message.User.id;

    // Bot is an automatic admin.
    if (myid == RPCMan->getBotDiscordID()) return true;

    for (auto adminid : GlobalConfig.General.Admins)
    {
        if (myid == adminid)
            return true;
    }
    return false;
}

void TIPBOT::globalHelpMenu(const UserMessage& message)
{
    std::vector<struct Command> Commands;

    for (auto & app : Apps)
    {
        if (app->isEnabled() || isUserAdmin(message))
        {
            auto * helpfunc = app->getHelpCommand();

            if (helpfunc)
            {
                Commands.emplace_back(*helpfunc);
            }
        }
    }
    SendMsg(message, generateHelpText("Tipbot Apps:", Commands, message));
}

void TIPBOT::SaveStats()
{
    DispatchMu.lock();
    std::ofstream out(PERFORMANCE_STATS_FILENAME, std::ios::trunc);
    if (out.is_open())
    {
        PLog->information("Saving performance stats to disk...");
        {
            cereal::JSONOutputArchive ar(out);
            ar(CEREAL_NVP(AppPerformanceStats));
        }
        out.close();
    }
    DispatchMu.unlock();
}

void TIPBOT::LoadStats()
{
    DispatchMu.lock();
    std::ifstream in(PERFORMANCE_STATS_FILENAME);
    if (in.is_open())
    {
        PLog->information("Loading performance stats from disk...");
        {
            cereal::JSONInputArchive ar(in);
            ar(CEREAL_NVP(AppPerformanceStats));
        }
        in.close();
    }
    DispatchMu.unlock();
}

void TIPBOT::dispatcher(const UserMessage& message, const struct Command & me, const std::shared_ptr<AppBaseClass> & ptr)
{
    Poco::Logger & tlog = Poco::Logger::get("CommandDispatch");
    GlobalConfig.General.Threads++;

    tlog.information("Thread Started! Threads: %?i", GlobalConfig.General.Threads);

    const ExecuteCommand cmdexe = { Poco::Timestamp(), message, me };

    DispatchMu.lock();
    runningCommands.insert(cmdexe);
    DispatchMu.unlock();

    if (!GlobalConfig.General.Shutdown)
    {
        try
        {
            if (me.opensWallet)
                ptr->setAccount(&RPCMan->getAccount(message.User.id));
            else  ptr->setAccount(nullptr);

            tlog.information("User %s (%s) issued command: %s", message.User.id_str, message.User.username, message.Message);

            me.func(this, message, me);
        }
        catch (const Poco::Exception & exp)
        {
            tlog.error("Poco Error: --- %s", std::string(exp.what()));
            SendMsg(message, "Poco Error: ---" + std::string(exp.what()) + " :cold_sweat:");
        }
        catch (AppGeneralException & exp)
        {
            tlog.error("App Error: --- %s: %s", std::string(exp.what()), exp.getGeneralError());
            SendMsg(message, std::string(exp.what()) + " --- " + exp.getGeneralError() + " :cold_sweat:");
        }
    }
    Poco::Timespan timeElapsed(Poco::Timestamp() - cmdexe.time_started);
    tlog.information("User %s (%s)'s command (%s) ended. Time elapsed: %?i ms", message.User.id_str, message.User.username, message.Message, timeElapsed.totalMilliseconds());

    DispatchMu.lock();
    auto & stat = AppPerformanceStats[me.name];
    stat.totalTime += timeElapsed.totalMilliseconds();
    stat.calls++;
    runningCommands.erase(cmdexe);
    DispatchMu.unlock();

    GlobalConfig.General.Threads--;

    tlog.information("Thread Stopped! Threads: %?i", GlobalConfig.General.Threads);
}

void TIPBOT::ProcessCommand(const UserMessage & message)
{
#ifndef NO_CHAISCRIPT
    // Script Hook. 
    ScriptEng.call_back(ecallback::OnMessage, { reinterpret_cast<const void*>(&message) });
#endif

    // Global help menu hook.
    if (message.Message == "!help") 
    {
        globalHelpMenu(message);
        return;
    }

    for (const auto & ptr : Apps)
    {
        if (ptr->isEnabled() || isUserAdmin(message))
        {
            // Application non-command function.
            ptr->run(message);

            if (!message.Message.empty() && message.Message.at(0) == '!')
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

                                if (TIPBOT::isCommandAllowedToBeExecuted(message, command))
                                    std::thread(&TIPBOT::dispatcher, this, message, command, ptr).detach(); // Create command thread
                            }
                            break;
                        }
                    }
                    catch (const Poco::Exception & exp)
                    {
                        PLog->error("Poco Error: --- %s", std::string(exp.what()));
                        SendMsg(message, "Poco Error: ---" + std::string(exp.what()) + " :cold_sweat:");
                    }
                    catch (AppGeneralException & exp)
                    {
                        PLog->error("App Error: --- %s: %s", std::string(exp.what()), exp.getGeneralError());
                        SendMsg(message, std::string(exp.what()) + " --- " + exp.getGeneralError() + " :cold_sweat:");
                    }
                }
            }
        }
    }
}

void TIPBOT::CommandParseError(const UserMessage& message, const Command& me)
{
    PLog->warning("User command error: User %s gave '%s' expected format '%s'", message.User.id_str, message.Message, me.params);
    SendMsg(message, Poco::format("Command Error --- Correct Usage: %s %s :cold_sweat:", me.name, me.params));
}

bool TIPBOT::isCommandAllowedToBeExecuted(const UserMessage& message, const Command& command)
{
    return message.ChannelPerm != AllowChannelTypes::Error && (!command.adminTools || (command.adminTools && (message.ChannelPerm == AllowChannelTypes::Private || message.ChannelPerm == AllowChannelTypes::CLI || command.ChannelPermission == AllowChannelTypes::Any) && TIPBOT::isUserAdmin(message)));
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
    SaveStats();
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
        broadcastMsg(data.Channel.id, message);
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

void TIPBOT::SendDirectMsg(const DiscordID& usr, std::string message)
{
    broadcastDirectMsg(usr, message);
}

std::vector<std::shared_ptr<AppBaseClass>>&   TIPBOT::getApps()
{
    return Apps;
}