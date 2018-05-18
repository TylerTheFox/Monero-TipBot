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
#include "Discord.h"
#include <memory>

#include <cereal/archives/json.hpp>
#include "Poco/Process.h"

#define RPC_DATABASE_FILENAME                   "RPCDATA.json"

class TIPBOT;

extern bool useOldConfig;

struct RPCProc
{
    RPCProc() : pid(0), RPCFail(0) {}
    RPCProc(const RPCProc & obj)
    {
        *this = obj;
    }

    Poco::Timestamp                 timestamp;
    unsigned int                    pid;
    RPC                             MyRPC;
    Account                         MyAccount;
    struct TransferList             Transactions;
    unsigned char                   RPCFail;

    RPCProc& operator=(const RPCProc & obj)
    {
        timestamp = obj.timestamp;
        pid = obj.pid;
        MyRPC = obj.MyRPC;
        MyAccount = obj.MyAccount;
        Transactions = obj.Transactions;
        return *this;
    }

    template <class Archive>
    void save(Archive & ar) const
    {
        Poco::Int64 val = timestamp.epochMicroseconds();
        ar(::cereal::make_nvp("timestamp", val));
    }

    template <class Archive>
    void load(Archive & ar)
    {
        if (GlobalConfig.About.major > 2 || GlobalConfig.About.major > 2 && GlobalConfig.About.minor > 0)
        {
            Poco::Int64 val = timestamp.epochMicroseconds();
            ar(::cereal::make_nvp("timestamp", val));
            timestamp = val;
        }
    }
};

// Mutex only needed for map and stack add/remove.
class RPCManager : public Poco::Runnable
{
public:
    RPCManager();
    ~RPCManager();

    void                                    setBotUser(DiscordID id);
    void                                    setDiscordPtr(TIPBOT* ptr);

    time_t                                  getTimeStarted(DiscordID id);
    Account &                               getAccount(DiscordID id);
    const struct TransferList               getTransfers(DiscordID id);
    const RPC&                              getRPC(DiscordID id);
    static const RPC&                       getGlobalBotRPC();
    static       Account &                  getGlobalBotAccount();
    const DiscordID &                       getBotDiscordID();
    std::uint64_t                           getTotalBalance();
    std::uint64_t                           getTotalUnlockedBalance();

    virtual void                            run();
    static std::shared_ptr<RPCProc>         manuallyCreateRPC(const std::string & walletname, unsigned short port);
    void                                    waitForRPCToRespond(DiscordID id, const RPC & rpc);
    void                                    restartWallet(DiscordID id);
    void                                    rescanAll();
    void                                    saveallWallets();

    void                                    save();
    void                                    load();
private:
    Poco::Mutex                             mu;
    unsigned short                          currPortNum;
    std::map<DiscordID, struct RPCProc>     RPCMap;
    DiscordID                               BotID;
    TIPBOT*                                 DiscordPtr;

    void                                    processNewTransactions();
    void                                    watchDog();
    bool                                    isRPCRunning(DiscordID id);
    struct RPCProc                          SpinUpNewRPC(DiscordID id, unsigned short port = 0);
    void                                    SpinDownRPC(DiscordID id);
    struct RPCProc &                        FindOldestRPC();
    void                                    SaveWallets();
    void                                    ReloadSavedRPCs();
    unsigned int                            LaunchRPC(unsigned short port);
};

extern std::unique_ptr<RPCManager>          RPCMan;
