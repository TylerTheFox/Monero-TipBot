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

#define CLASS_RESOLUTION(x) std::bind(&Projects::x, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
Projects::Projects(TIPBOT * DPTR) : enabled(true), PLog(nullptr), DiscordPtr(DPTR)
{
    Commands =
    {
        // User Commands 
        // Command            Function                                        Params                            Wallet  Admin   Allowed Channel
        { "!projects",        CLASS_RESOLUTION(Help),                         "",                               false,  false,  AllowChannelTypes::Any },
        { "!fundproject",     CLASS_RESOLUTION(FundProject),                  "[amount] [project]",             false,  false, AllowChannelTypes::Any },
        { "!projectdonors",   CLASS_RESOLUTION(ProjectDonors),                "[project]",                      false,  false, AllowChannelTypes::Any },
        { "!listprojects",    CLASS_RESOLUTION(ListProjects),                 "",                               false,  false, AllowChannelTypes::Any },
        { "!viewstatus",      CLASS_RESOLUTION(ViewStatus),                   "",                               false,  false, AllowChannelTypes::Any },

        // Admin
        { "!create",          CLASS_RESOLUTION(Create),                       "[project] [description] [goal]", false,  true,  AllowChannelTypes::Private },
        { "!delete",          CLASS_RESOLUTION(Delete),                       "[project]",                      false,  true,  AllowChannelTypes::Private },
        { "!grantuser",       CLASS_RESOLUTION(GrantUser),                    "[project] [user]",               false,  true,  AllowChannelTypes::Any },
        { "!suspendproject",  CLASS_RESOLUTION(SuspendProject),               "[project]",                      false,  true,  AllowChannelTypes::Private },
    };
    PLog = &Poco::Logger::get("Projects");
}

void Projects::save()
{
}

void Projects::load()
{

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
        const std::uint64_t & goal = Poco::NumberParser::parseFloat(cmd[3]);;
        
        if (ProjectMap.count(name))
        {
            PLog->warning("Project %s already exists!", name);
            DiscordPtr->SendMsg(message, "Error project already exists!");
            return;
        }

        PLog->information("Creating project %s with goal %?i on RPC port %?i", name, goal, GlobalConfig.RPCManager.starting_port_number - 2 - ProjectMap.size());
        ProjectMap[name] = { goal, RPCManager::manuallyCreateRPC(getFilename(name), GlobalConfig.RPCManager.starting_port_number - 2 - ProjectMap.size()) };

        DiscordPtr->SendMsg(message, "Project created successfully!");
    }
}

void Projects::Delete(TIPBOT * DiscordPtr, const UserMessage & message, const Command & me)
{
}

void Projects::GrantUser(TIPBOT * DiscordPtr, const UserMessage & message, const Command & me)
{
}

void Projects::SuspendProject(TIPBOT * DiscordPtr, const UserMessage & message, const Command & me)
{
}

void Projects::FundProject(TIPBOT * DiscordPtr, const UserMessage & message, const Command & me)
{
}

void Projects::ProjectDonors(TIPBOT * DiscordPtr, const UserMessage & message, const Command & me)
{
}

void Projects::ListProjects(TIPBOT * DiscordPtr, const UserMessage & message, const Command & me)
{
}

void Projects::ViewStatus(TIPBOT * DiscordPtr, const UserMessage & message, const Command & me)
{
}

const std::string Projects::getFilename(const std::string & projectname)
{
    return Poco::format("PROJECT-%s", projectname);
}
