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
#include <map>
#include "types.h"
#include "RPC.h"
#include "Poco/Timestamp.h"
#include "Account.h"
#include "Poco/Runnable.h"
#include "Tipbot.h"
#include <memory>
#include "Config.h"

#include <cereal/archives/json.hpp>
#include "Poco/Process.h"
#include "Poco/Logger.h"
#include "Poco/AutoPtr.h"

#define RPC_DATABASE_FILENAME                   "RPCDATA.json"

class TIPBOT;

struct RPCProc
{
    RPCProc() : pid(0) {}

    Poco::Timestamp                 timestamp;
    unsigned int                    pid;
    RPC                             MyRPC;
};

// Mutex only needed for map and stack add/remove.
class RPCManager : public Poco::Runnable
{
public:
    RPCManager();
    ~RPCManager();
    RPCManager(const RPCManager &);

    void                                    setBotUser(DiscordID id);
    void                                    setDiscordPtr(TIPBOT* ptr);

    Account &                               getAccount(DiscordID id);
    Account &                               getAccount(const std::string & name);
    const struct TransferList               getTransfers(DiscordID id);
    static Account&                         getGlobalBotAccount();
    const DiscordID &                       getBotDiscordID();



    virtual void                            run();
private:
    Poco::Logger*                           PLog;
    Poco::Mutex                             mu;
    DiscordID                               BotID;
    TIPBOT*                                 DiscordPtr;
    RPCProc                                 MainProc;

    struct RPCProc                          SpinUpNewRPC(DiscordID id, unsigned short port = 0);
    void                                    SaveWallet();
    unsigned int                            LaunchRPC(unsigned short port);
    static std::shared_ptr<RPCProc>         manuallyCreateRPC(const std::string & walletname, unsigned short port);
    void                                    waitForRPCToRespond(DiscordID id, const RPC & rpc);

};

extern std::unique_ptr<RPCManager>          RPCMan;
