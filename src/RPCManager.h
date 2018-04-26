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

#include <cereal/archives/json.hpp>
#include "Poco/Process.h"

#define                        RPC_DATABASE_FILENAME                "RPCDATA.json"
#define                        STARTING_PORT_NUMBER                 11000
#define                        MAX_RPC_LIMIT                        100
#define                        BLOCKCHAIN_SAVE_TIME                 5    // in minutes.
#define                        SEARCH_FOR_NEW_TRANSACTIONS_TIME     10    // in seconds
#define                        RPC_WALLETS_SAVE_TIME                60    // in seconds
class ITNS_TIPBOT;

struct RPCProc
{
    RPCProc() : pid(0) {}
    RPCProc(const RPCProc & obj)
    {
        *this = obj;
    }

    Poco::Timestamp                 timestamp;
    unsigned int                    pid;
    RPC                             MyRPC;
    Account                         MyAccount;
    struct TransferList             Transactions;

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
    void serialize(Archive & ar)
    {
        ar(CEREAL_NVP(MyRPC));
    }
};

// Mutex only needed for map and stack add/remove.
class RPCManager : public Poco::Runnable
{
public:
    RPCManager();
    ~RPCManager();
    void                                    setBotUser(DiscordID id);
    void                                    setDiscordPtr(ITNS_TIPBOT* ptr);
    time_t                                  getTimeStarted(DiscordID id);
    Account &                               getAccount(DiscordID id);
    const struct TransferList               getTransfers(DiscordID id);

    virtual void                            run();
    void                                    processNewTransactions();

    static const RPC&                       getGlobalBotRPC();
    static       Account &                  getGlobalBotAccount();

    void                                    save();
    void                                    load();
private:
    Poco::Mutex                             mu;
    unsigned short                          currPortNum;
    static struct RPCProc                   BotRPCProc;
    std::map<DiscordID, struct RPCProc>     RPCMap;
    ITNS_TIPBOT*                            DiscordPtr;

    bool                                    isRPCRunning(DiscordID id);
    struct RPCProc                          SpinUpNewRPC(DiscordID id);
    void                                    SpinDownRPC(DiscordID id);
    struct RPCProc &                        FindOldestRPC();
    void                                    SaveWallets();
    void                                    ReloadSavedRPCs();
    unsigned int                            LaunchRPC(unsigned short port);
    void                                    waitForRPCToRespond(DiscordID id, const RPC & rpc);
};

extern RPCManager                           RPCMan;