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
#include "Projects.h"
#include "Poco/StringTokenizer.h"
#include "Language.h"
#include <fstream>
#include "cereal/cereal.hpp"
#include "cereal/archives/json.hpp"
#include "cereal/types/map.hpp"

#define CLASS_RESOLUTION(x) std::bind(&Projects::x, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
Projects::Projects(TIPBOT * DPTR) : enabled(true), PLog(nullptr), DiscordPtr(DPTR)
{
    Commands =
    {
        // User Commands 
        // Command            Function                                        Params                            Wallet  Admin   Allowed Channel
        { "!projects",        CLASS_RESOLUTION(Help),                         "",                               false,  false,  AllowChannelTypes::Any },
        { "!fundproject",     CLASS_RESOLUTION(FundProject),                  "[amount] [project]",             false,  false,  AllowChannelTypes::Any },
        { "!listprojects",    CLASS_RESOLUTION(ListProjects),                 "",                               false,  false,  AllowChannelTypes::Any },
        { "!viewstatus",      CLASS_RESOLUTION(ViewStatus),                   "[project]",                      false,  false,  AllowChannelTypes::Any },
        { "!projectaddress",  CLASS_RESOLUTION(ProjectAddress),               "[project]",                      false,  false,  AllowChannelTypes::Any },

        // Admin
        { "!create",          CLASS_RESOLUTION(Create),                       "[project] [description] [goal]", false,  true,   AllowChannelTypes::Private },
        { "!delete",          CLASS_RESOLUTION(Delete),                       "[project]",                      false,  true,   AllowChannelTypes::Private },
        { "!grantuser",       CLASS_RESOLUTION(GrantUser),                    "[project] [user]",               false,  true,   AllowChannelTypes::Any },
        { "!toggleproject",   CLASS_RESOLUTION(ToggleProject),                "[project]",                      false,  true,   AllowChannelTypes::Private },
    };
    PLog = &Poco::Logger::get("Projects");
}

Projects::~Projects()
{
    for (auto & proj : ProjectMap)
    {
        try
        {
            proj.second.RPC->MyRPC.store();
            proj.second.RPC->MyRPC.stopWallet();
        }
        catch (...)
        {

        }
    }
}

void Projects::save()
{
    std::ofstream out(PROJECTS_SAVE_FILE, std::ios::trunc);
    if (out.is_open())
    {
        PLog->information("Saving projects data to disk...");
        {
            cereal::JSONOutputArchive ar(out);
            ar(CEREAL_NVP(ProjectMap));
        }
        out.close();
    }
}

void Projects::load()
{
    std::ifstream in(PROJECTS_SAVE_FILE);
    if (in.is_open())
    {
        PLog->information("Loading projects data from disk...");
        {
            cereal::JSONInputArchive ar(in);
            ar(CEREAL_NVP(ProjectMap));
        }
        in.close();

        auto i = GlobalConfig.RPCManager.starting_port_number - 2;
        for (auto & proj : ProjectMap)
            proj.second.RPC = RPCManager::manuallyCreateRPC(getFilename(proj.first), i--);
    }
}

void Projects::setAccount(Account*)
{
    // Do nothing, we construct this parameter since its pure virtual and we dont need it in this class.
}

iterator Projects::begin()
{
    return Commands.begin();
}

const_iterator Projects::begin() const
{
    return Commands.begin();
}

const_iterator Projects::cbegin() const
{
    return Commands.cbegin();
}

iterator Projects::end()
{
    return Commands.end();
}

const_iterator Projects::end() const
{
    return Commands.end();
}

const_iterator Projects::cend() const
{
    return Commands.cend();
}

void Projects::Help(TIPBOT * DiscordPtr, const UserMessage & message, const Command & me)
{
    const auto helpStr = TIPBOT::generateHelpText("Projects Help Menu", Commands, message);
    DiscordPtr->SendMsg(message, helpStr);
}

void Projects::Create(TIPBOT * DiscordPtr, const UserMessage & message, const Command & me)
{
    Poco::StringTokenizer cmd(message.Message, " ");

    if (cmd.count() < 4)
        DiscordPtr->CommandParseError(message, me);
    else
    {
        const std::string & name = cmd[1];
        const std::string & description = cmd[2];
        const std::uint64_t & goal = Poco::NumberParser::parseFloat(cmd[3]) * GlobalConfig.RPC.coin_offset;

        if (ProjectMap.count(name))
        {
            PLog->warning("Project %s already exists!", name);
            DiscordPtr->SendMsg(message, "Error project already exists!");
            return;
        }

        if (!goal)
        {
            PLog->warning("Project goal cannot be zero!");
            DiscordPtr->SendMsg(message, "Project goal cannot be zero!");
            return;
        }

        PLog->information("Creating project %s with goal %?i %s on RPC port %?i", name, goal, GlobalConfig.RPC.coin_abbv, GlobalConfig.RPCManager.starting_port_number - 2 - ProjectMap.size());
        ProjectMap[name] = { description, goal, false, RPCManager::manuallyCreateRPC(getFilename(name), GlobalConfig.RPCManager.starting_port_number - 2 - ProjectMap.size()) };
        save();
        DiscordPtr->SendMsg(message, "Project created successfully!");
    }
}

void Projects::Delete(TIPBOT * DiscordPtr, const UserMessage & message, const Command & me)
{
    Poco::StringTokenizer cmd(message.Message, " ");

    if (cmd.count() < 2)
        DiscordPtr->CommandParseError(message, me);
    else
    {
        const std::string & name = cmd[1];

        if (!ProjectMap.count(name))
        {
            PLog->warning("Project %s does not exists!", name);
            DiscordPtr->SendMsg(message, "Error project doesn't exist!");
            return;
        }

        const auto & proj = ProjectMap[name];

        proj.RPC->MyRPC.store();
        proj.RPC->MyRPC.stopWallet();

        ProjectMap.erase(name);
        save();
        DiscordPtr->SendMsg(message, "Project deleted successfully!");
    }
}

void Projects::GrantUser(TIPBOT * DiscordPtr, const UserMessage & message, const Command & me)
{
    Poco::StringTokenizer cmd(message.Message, " ");

    if (cmd.count() < 2 || message.Mentions.size() != 1)
        DiscordPtr->CommandParseError(message, me);
    else
    {
        const std::string & name = cmd[1];

        if (!ProjectMap.count(name))
        {
            PLog->warning("Project %s does not exists!", name);
            DiscordPtr->SendMsg(message, "Error project doesn't exist!");
            return;
        }

        const auto & proj = ProjectMap[name];
        const auto tx = proj.RPC->MyAccount.transferAllMoneytoAnotherDiscordUser(message.Mentions[0].id);

        DiscordPtr->SendMsg(message, Poco::format("Transfering all funds from project %s to user %s with tx hash %s", name, message.Mentions[0].username, tx.tx_hash));
    }
}

void Projects::ToggleProject(TIPBOT * DiscordPtr, const UserMessage & message, const Command & me)
{
    Poco::StringTokenizer cmd(message.Message, " ");

    if (cmd.count() < 2)
        DiscordPtr->CommandParseError(message, me);
    else
    {
        const std::string & name = cmd[1];

        if (!ProjectMap.count(name))
        {
            PLog->warning("Project %s does not exists!", name);
            DiscordPtr->SendMsg(message, "Error project doesn't exist!");
            return;
        }

        auto & proj = ProjectMap[name];
        proj.Suspended = !proj.Suspended;

        DiscordPtr->SendMsg(message, Poco::format("Project Status: %b", proj.Suspended));
    }
}

void Projects::FundProject(TIPBOT * DiscordPtr, const UserMessage & message, const Command & me)
{
    Poco::StringTokenizer cmd(message.Message, " ");

    if (cmd.count() < 3)
        DiscordPtr->CommandParseError(message, me);
    else
    {
        const auto & amount = Poco::NumberParser::parseFloat(cmd[1]);
        const std::string & name = cmd[2];

        if (!ProjectMap.count(name))
        {
            PLog->warning("Project %s does not exists!", name);
            DiscordPtr->SendMsg(message, "Error project doesn't exist!");
            return;
        }

        const auto & proj = ProjectMap[name];
        if (!proj.Suspended)
        {
            auto & usr = RPCMan->getAccount(message.User.id);
            const auto tx = usr.transferMoneyToAddress(static_cast<std::uint64_t>(amount * GlobalConfig.RPC.coin_offset), proj.RPC->MyRPC.getAddress());

            DiscordPtr->SendMsg(message, Poco::format("Sending %0.8f %s to project %s with tx hash %s", amount, GlobalConfig.RPC.coin_abbv, name, tx.tx_hash));
        }
        else DiscordPtr->SendMsg(message, "Project Suspended!");
    }
}

void Projects::ListProjects(TIPBOT * DiscordPtr, const UserMessage & message, const Command & me)
{
    std::stringstream ss;

    ss << "Project, Descrption, Goal, Percentage Funded, Suspended\\n";
    ss << "```";
    if (ProjectMap.size())
    {
        for (const auto & proj : ProjectMap)
        {
            proj.second.RPC->MyAccount.resyncAccount();
            ss << proj.first << ", " << proj.second.Description << ", " << proj.second.Goal / GlobalConfig.RPC.coin_offset << " " << GlobalConfig.RPC.coin_abbv << ", " << (proj.second.RPC->MyAccount.getBalance() / static_cast<double>(proj.second.Goal)) * 100.0 << "%, " << (proj.second.Suspended ? "Yes" : "No") << "\\n";
        }
    }
    else ss << "No Projects!";
    ss << "```";
    DiscordPtr->SendMsg(message, ss.str());
}

void Projects::ViewStatus(TIPBOT * DiscordPtr, const UserMessage & message, const Command & me)
{
    Poco::StringTokenizer cmd(message.Message, " ");

    if (cmd.count() < 2)
        DiscordPtr->CommandParseError(message, me);
    else
    {
        const std::string & name = cmd[1];

        if (!ProjectMap.count(name))
        {
            PLog->warning("Project %s does not exists!", name);
            DiscordPtr->SendMsg(message, "Error project doesn't exist!");
            return;
        }
        const auto & proj = ProjectMap[name];

        proj.RPC->MyAccount.resyncAccount();

        DiscordPtr->SendMsg(message, Poco::format("%s:```Description: %s\\nBalance: %0.8f %s\\nUnlocked Balance: %0.8f %s\\nGoal: %0.8f %s\\nPercentage Complete: %0.2f%%```",
            name,
            proj.Description,
            proj.RPC->MyAccount.getBalance() / GlobalConfig.RPC.coin_offset,
            GlobalConfig.RPC.coin_abbv,
            proj.RPC->MyAccount.getUnlockedBalance() / GlobalConfig.RPC.coin_offset,
            GlobalConfig.RPC.coin_abbv,
            proj.Goal / GlobalConfig.RPC.coin_offset,
            GlobalConfig.RPC.coin_abbv,
            (proj.RPC->MyAccount.getBalance() / static_cast<double>(proj.Goal)) * 100.0)
        );
    }
}

void Projects::ProjectAddress(TIPBOT * DiscordPtr, const UserMessage & message, const Command & me)
{
    Poco::StringTokenizer cmd(message.Message, " ");

    if (cmd.count() < 2)
        DiscordPtr->CommandParseError(message, me);
    else
    {
        const std::string & name = cmd[1];

        if (!ProjectMap.count(name))
        {
            PLog->warning("Project %s does not exists!", name);
            DiscordPtr->SendMsg(message, "Error project doesn't exist!");
            return;
        }
        const auto & proj = ProjectMap[name];

        proj.RPC->MyAccount.resyncAccount();

        DiscordPtr->SendMsg(message, Poco::format("Project Direct Address: %s", proj.RPC->MyAccount.getMyAddress()));
    }
}

const std::string Projects::getFilename(const std::string & projectname)
{
    return Poco::format("PROJECT-%s", projectname);
}
