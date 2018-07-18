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
#pragma once
#include "Tipbot.h"
#include "Account.h"
#include "Util.h"
#include "AppBaseClass.h"
#include "Poco/Logger.h"
#include "Poco/AutoPtr.h"
#include "RPCManager.h"
/*
******************************************************** ISSUE #1 ********************************************************

Hey there, been watching your repo, great job by the way. I was curious if there was a way to fund a wallet for bounties.

For example to create bounty,
!createbounty <TotalAmount> <Request> <Description>

Bot Response:
"<Username> created a bounty for <Description> for <TotalAmount> ITNS"

Then, people would donate to the wallet by using:
!donate <Request> <Amount>

Bot Response:
"<Username> donated <Amount> ITNS for <Description>. You can contribute to the cause by using !donate <Request> amount"

To show bounties:
!showbounty ALL
to show all
and:
!showbounty <request>
to see a specific bounty

Part of the wallet creation would have to be creating a viewkey/ allowing the users to see how much is funded.

*****************************************************************************************************************

Notes:
* No time limit
* Set a goal to accomplish
* Track goal(s)
* Allowed to exceed 100% funded.
* Operator will manaully release coins.
* Not required to give out view key. Bot will have a !balance like command for each project.
* Top Donors list.
* Will be called Projects

Possible Commands:
!create [project] [description] [goal]  	    ********** ADMIN
!delete [project] 	 		                    ********** ADMIN
!grantuser [project] [user]  		            ********** ADMIN
!suspendproject [project] 		                ********** ADMIN
!fundproject [amount] [project]
(maybe) !projectdonors [project]
!listprojects
!viewstatus

*****************************************************************************************************************
*/

struct Project
{
    std::uint64_t               Goal;
    std::shared_ptr<RPCProc>    RPC;

    template <class Archive>
    void serialize(Archive & ar)
    {
        ar(
            CEREAL_NVP(Goal)
        );
    }
};

#define    PROJECTS_SAVE_FILE       "PROJECTS.JSON"
class Projects : public AppBaseClass
{
public:
    Projects(TIPBOT * DPTR);
    virtual ~Projects() = default;

    void                                save();
    void                                load();
    void                                setAccount(Account *);
    iterator                            begin();
    const_iterator                      begin() const;
    const_iterator                      cbegin() const;

    iterator                            end();
    const_iterator                      end() const;
    const_iterator                      cend() const;

    void                                Help(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void                                Create(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void                                Delete(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void                                GrantUser(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void                                SuspendProject(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void                                FundProject(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void                                ProjectDonors(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void                                ListProjects(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void                                ViewStatus(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);

private:
    std::map<std::string, Project>      ProjectMap;
    TIPBOT *                            DiscordPtr;
    Poco::Logger*                       PLog;
    bool                                enabled;
    std::vector<struct Command>         Commands;

    const std::string                   getFilename(const std::string & projectname);

};
