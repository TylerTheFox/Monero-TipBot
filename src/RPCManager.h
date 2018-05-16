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

#define                        RPC_DATABASE_FILENAME                "RPCDATA.json"
#define                        STARTING_PORT_NUMBER                 11000
#define                        MAX_RPC_LIMIT                        200
#define                        RPC_ERROR_GIVEUP                     3
#define                        BLOCKCHAIN_SAVE_TIME                 (5/* Minutes */*60)
#define                        SEARCH_FOR_NEW_TRANSACTIONS_TIME     (10/*In Seconds*/)
#define                        RPC_WALLETS_SAVE_TIME                (60/*In Seconds*/)
#define                        RPC_WALLET_WATCHDOG                  (10/*In Minutes*/*60)

class ITNS_TIPBOT;

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
    ITNS_TIPBOT*                            DiscordPtr;

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

extern RPCManager                           RPCMan;
