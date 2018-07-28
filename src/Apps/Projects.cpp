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
#include "../Core/Language.h"
#include <fstream>
#include "cereal/cereal.hpp"
#include "cereal/archives/json.hpp"
#include "cereal/types/map.hpp"
#include "../Core/Util.h"

#define CLASS_RESOLUTION(x) std::bind(&Projects::x, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
Projects::Projects(TIPBOT * DPTR) : enabled(true), PLog(nullptr), DiscordPtr(DPTR), PortCount(GlobalConfig.RPCManager.starting_port_number - 2)
{
    Commands =
    {
        // User Commands 
        // Command            Function                                        Params                                            Wallet  Admin   Allowed Channel
        { "!projects",        CLASS_RESOLUTION(Help),                         "",                                               false,  false,  AllowChannelTypes::Any },
        { "!fundproject",     CLASS_RESOLUTION(FundProject),                  "[amount] \\\"[project]\\\"",                     false,  false,  AllowChannelTypes::Any },
        { "!listprojects",    CLASS_RESOLUTION(ListProjects),                 "",                                               false,  false,  AllowChannelTypes::Any },
        { "!viewstatus",      CLASS_RESOLUTION(ViewStatus),                   "\\\"[project]\\\"",                              false,  false,  AllowChannelTypes::Any },
        { "!projectaddress",  CLASS_RESOLUTION(ProjectAddress),               "\\\"[project]\\\"",                              false,  false,  AllowChannelTypes::Any },

        // Admin
        { "!create",          CLASS_RESOLUTION(Create),                       "\\\"[project]\\\" \\\"[description]\\\" [goal]", false,  true,   AllowChannelTypes::Private },
        { "!delete",          CLASS_RESOLUTION(Delete),                       "\\\"[project]\\\"",                              false,  true,   AllowChannelTypes::Private },
        { "!grantuser",       CLASS_RESOLUTION(GrantUser),                    "\\\"[project]\\\" [user]",                       false,  true,   AllowChannelTypes::Any },
        { "!toggleproject",   CLASS_RESOLUTION(ToggleProject),                "\\\"[project]\\\"",                              false,  true,   AllowChannelTypes::Private },
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

        PortCount = GlobalConfig.RPCManager.starting_port_number - 2;
        for (auto & proj : ProjectMap)
            proj.second.RPC = RPCManager::manuallyCreateRPC(getFilename(proj.first), PortCount--);
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
    const auto helpStr = TIPBOT::generateHelpText(GETSTR(DiscordPtr->getUserLang(message.User.id), "PROJECTS_HELP"), Commands, message);
    DiscordPtr->SendMsg(message, helpStr);
}

void Projects::Create(TIPBOT * DiscordPtr, const UserMessage & message, const Command & me)
{
    Poco::StringTokenizer cmd(message.Message, " ");

    if (cmd.count() < 4)
        DiscordPtr->CommandParseError(message, me);
    else
    {
        unsigned int ret_idx;
        std::string name;
        std::string description;

        bool nameValid = Util::parseQuotedString(cmd, 1, name, ret_idx);
        if (nameValid)
        {
            bool descriptionValid = Util::parseQuotedString(cmd, ret_idx, description, ret_idx);

            if (descriptionValid)
            {
                const std::uint64_t & goal = Poco::NumberParser::parseFloat(cmd[ret_idx]) * GlobalConfig.RPC.coin_offset;

                if (ProjectMap.count(name))
                {
                    PLog->warning("Project %s already exists!", name);
                    DiscordPtr->SendMsg(message, GETSTR(DiscordPtr->getUserLang(message.User.id), "PROJECTS_ERROR_EXISTS"));
                    return;
                }

                if (!goal)
                {
                    PLog->warning("Project goal cannot be zero!");
                    DiscordPtr->SendMsg(message, GETSTR(DiscordPtr->getUserLang(message.User.id), "PROJECTS_ERROR_GOAL_ZERO"));
                    return;
                }

                PLog->information("Creating project %s with goal %?i %s on RPC port %?i", name, goal, GlobalConfig.RPC.coin_abbv, GlobalConfig.RPCManager.starting_port_number - 2 - PortCount);
                ProjectMap[name] = { description, goal, false, RPCManager::manuallyCreateRPC(getFilename(name), GlobalConfig.RPCManager.starting_port_number - 2 - PortCount--) };
                save();
                DiscordPtr->SendMsg(message, GETSTR(DiscordPtr->getUserLang(message.User.id), "PROJECTS_CREATED_SUCCESS"));
            }
            else DiscordPtr->CommandParseError(message, me);
        }
        else DiscordPtr->CommandParseError(message, me);
    }
}

void Projects::Delete(TIPBOT * DiscordPtr, const UserMessage & message, const Command & me)
{
    Poco::StringTokenizer cmd(message.Message, " ");

    if (cmd.count() < 2)
        DiscordPtr->CommandParseError(message, me);
    else
    {
        std::string  name;
        unsigned int ret_idx;
        bool nameValid = Util::parseQuotedString(cmd, 1, name, ret_idx);
        if (nameValid)
        {
            if (!ProjectMap.count(name))
            {
                PLog->warning("Project %s does not exists!", name);
                DiscordPtr->SendMsg(message, GETSTR(DiscordPtr->getUserLang(message.User.id), "PROJECTS_NOT_EXIST"));
                return;
            }

            const auto & proj = ProjectMap[name];

            proj.RPC->MyRPC.store();
            proj.RPC->MyRPC.stopWallet();

            ProjectMap.erase(name);
            save();
            DiscordPtr->SendMsg(message, GETSTR(DiscordPtr->getUserLang(message.User.id), "PROJECTS_DELETE_SUCCESS"));
        }
    }
}

void Projects::GrantUser(TIPBOT * DiscordPtr, const UserMessage & message, const Command & me)
{
    Poco::StringTokenizer cmd(message.Message, " ");

    if (cmd.count() < 2 || message.Mentions.size() != 1)
        DiscordPtr->CommandParseError(message, me);
    else
    {
        std::string  name;
        unsigned int ret_idx;
        bool nameValid = Util::parseQuotedString(cmd, 1, name, ret_idx);
        if (nameValid)
        {
            if (!ProjectMap.count(name))
            {
                PLog->warning("Project %s does not exists!", name);
                DiscordPtr->SendMsg(message, GETSTR(DiscordPtr->getUserLang(message.User.id), "PROJECTS_NOT_EXIST"));
                return;
            }

            const auto & proj = ProjectMap[name];
            const auto tx = proj.RPC->MyAccount.transferAllMoneytoAnotherDiscordUser(message.Mentions[0].id);

            DiscordPtr->SendMsg(message, Poco::format(GETSTR(DiscordPtr->getUserLang(message.User.id), "PROJECTS_GRANT_SUCCESS"), name, message.Mentions[0].username, tx.tx_hash));
        }
        else  DiscordPtr->CommandParseError(message, me);
    }
}

void Projects::ToggleProject(TIPBOT * DiscordPtr, const UserMessage & message, const Command & me)
{
    Poco::StringTokenizer cmd(message.Message, " ");

    if (cmd.count() < 2)
        DiscordPtr->CommandParseError(message, me);
    else
    {
        std::string  name;
        unsigned int ret_idx;
        bool nameValid = Util::parseQuotedString(cmd, 1, name, ret_idx);
        if (nameValid)
        {
            if (!ProjectMap.count(name))
            {
                PLog->warning("Project %s does not exists!", name);
                DiscordPtr->SendMsg(message, GETSTR(DiscordPtr->getUserLang(message.User.id), "PROJECTS_NOT_EXIST"));
                return;
            }

            auto & proj = ProjectMap[name];
            proj.Suspended = !proj.Suspended;

            DiscordPtr->SendMsg(message, Poco::format("Project Status: %b", !proj.Suspended));
        }
        else DiscordPtr->CommandParseError(message, me);
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
        std::string name;
        unsigned int ret_idx;
        bool nameValid = Util::parseQuotedString(cmd, 2, name, ret_idx);

        if (nameValid)
        {
            if (!ProjectMap.count(name))
            {
                PLog->warning("Project %s does not exists!", name);
                DiscordPtr->SendMsg(message, GETSTR(DiscordPtr->getUserLang(message.User.id), "PROJECTS_NOT_EXIST"));
                return;
            }

            const auto & proj = ProjectMap[name];
            if (!proj.Suspended)
            {
                auto & usr = RPCMan->getAccount(message.User.id);
                const auto tx = usr.transferMoneyToAddress(static_cast<std::uint64_t>(amount * GlobalConfig.RPC.coin_offset), proj.RPC->MyRPC.getAddress());

                DiscordPtr->SendMsg(message, Poco::format(GETSTR(DiscordPtr->getUserLang(message.User.id), "PROJECTS_FUND_SUCCESS"), amount, GlobalConfig.RPC.coin_abbv, name, tx.tx_hash));
            }
            else DiscordPtr->SendMsg(message, GETSTR(DiscordPtr->getUserLang(message.User.id), "PROJECTS_SUSPENDED"));
        }
        else DiscordPtr->CommandParseError(message, me);
    }
}

void Projects::ListProjects(TIPBOT * DiscordPtr, const UserMessage & message, const Command & me)
{
    std::stringstream ss;

    ss << GETSTR(DiscordPtr->getUserLang(message.User.id), "PROJECTS_LIST_HEADER");
    ss << "```";
    if (ProjectMap.size())
    {
        for (const auto & proj : ProjectMap)
        {
            proj.second.RPC->MyAccount.resyncAccount();
            ss << "\\\"" << proj.first << "\\\", \\\"" << proj.second.Description << "\\\", " << proj.second.Goal / GlobalConfig.RPC.coin_offset << " " << GlobalConfig.RPC.coin_abbv << ", " << (proj.second.RPC->MyAccount.getBalance() / static_cast<double>(proj.second.Goal)) * 100.0 << "%, " << (proj.second.Suspended ? "Yes" : "No") << "\\n";
        }
    }
    else ss << GETSTR(DiscordPtr->getUserLang(message.User.id), "PROJECTS_NO_PROJECTS");
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
        std::string  name;
        unsigned int ret_idx;
        bool nameValid = Util::parseQuotedString(cmd, 1, name, ret_idx);
        if (nameValid)
        {
            if (!ProjectMap.count(name))
            {
                PLog->warning("Project %s does not exists!", name);
                DiscordPtr->SendMsg(message, GETSTR(DiscordPtr->getUserLang(message.User.id), "PROJECTS_NOT_EXIST"));
                return;
            }
            const auto & proj = ProjectMap[name];

            proj.RPC->MyAccount.resyncAccount();

            DiscordPtr->SendMsg(message, Poco::format(GETSTR(DiscordPtr->getUserLang(message.User.id), "PROJECTS_VIEW_STATUS_LIST"),
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
        else DiscordPtr->CommandParseError(message, me);
    }
}

void Projects::ProjectAddress(TIPBOT * DiscordPtr, const UserMessage & message, const Command & me)
{
    Poco::StringTokenizer cmd(message.Message, " ");

    if (cmd.count() < 2)
        DiscordPtr->CommandParseError(message, me);
    else
    {
        std::string  name;

        unsigned int ret_idx;
        bool nameValid = Util::parseQuotedString(cmd, 1, name, ret_idx);

        if (nameValid)
        {
            if (!ProjectMap.count(name))
            {
                PLog->warning("Project %s does not exists!", name);
                DiscordPtr->SendMsg(message, GETSTR(DiscordPtr->getUserLang(message.User.id), "PROJECTS_NOT_EXIST"));
                return;
            }
            const auto & proj = ProjectMap[name];

            proj.RPC->MyAccount.resyncAccount();

            DiscordPtr->SendMsg(message, Poco::format(GETSTR(DiscordPtr->getUserLang(message.User.id), "PROJECTS_DIRECT_ADDRESS"), name, proj.RPC->MyAccount.getMyAddress()));
        }
        else DiscordPtr->CommandParseError(message, me);
    }
}

const std::string Projects::getFilename(const std::string & projectname)
{
    return Poco::format("PROJECT-%s", projectname);
}
