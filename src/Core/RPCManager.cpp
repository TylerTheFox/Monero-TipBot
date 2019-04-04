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
#include "RPCManager.h"
#include "Poco/Process.h"
#include "Poco/Thread.h"
#include "Poco/Timespan.h"
#include "Util.h"
#include <cassert>
#include "Tipbot.h"
#include "RPCException.h"
#include "Poco/File.h"
#include "cereal/types/map.hpp"
#include "cereal/types/set.hpp"
#include <fstream>
#include "AccountException.h"
#include "Poco/ScopedLock.h"
#include "Config.h"
#include "Poco/File.h"

std::unique_ptr<RPCManager>      RPCMan;

RPCManager::RPCManager() : DiscordPtr(nullptr), BotID(0), PLog(nullptr)
{
    PLog = &Poco::Logger::get("RPCManager");

    // Check if RPC Exists.
    std::string filename = GlobalConfig.RPC.filename;
#ifdef WIN32
    filename += ".exe";
#endif
    if (!Poco::File(filename).exists())
        throw RPCGeneralError("-1", "RPC Binary Not Found!");

    // Launch RPC
    MainProc = SpinUpNewRPC(0, GlobalConfig.RPCManager.starting_port_number);
}

RPCManager::RPCManager(const RPCManager & rhs)
{
    PLog = rhs.PLog;
    BotID = rhs.BotID;
    DiscordPtr = rhs.DiscordPtr;
}

RPCManager::~RPCManager()
{
    MainProc.MyRPC.closeWallet();
}

void RPCManager::setBotUser(DiscordID id)
{
    BotID = id;
    try
    {
        getAccount(BotID); // Just open the wallet and discard the result.
    }
    catch (AppGeneralException & exp)
    {
        PLog->error("App Error: %s --- %s", std::string(exp.what()), exp.getGeneralError());
    }
}

void RPCManager::setDiscordPtr(TIPBOT* ptr)
{
    DiscordPtr = ptr;
}

Account& RPCManager::getAccount(DiscordID id)
{
    Poco::Mutex::ScopedLock lock(mu);
    return Account();
}

Account & RPCManager::getAccount(const std::string & name)
{
    Poco::Mutex::ScopedLock lock(mu);
    return Account();
}

Account& RPCManager::getGlobalBotAccount()
{
    return Account();
}

const TransferList RPCManager::getTransfers(DiscordID id)
{
    //if (RPCMap.count(id))
    //    return RPCMap[id].Transactions;
    return {};
}

void RPCManager::run()
{
    time_t  currTime = Poco::Timestamp().epochTime();
    time_t  
        walletTime = currTime + GlobalConfig.RPCManager.blockchain_save_time;

    GlobalConfig.General.Threads++;

    PLog->information("Thread Started! Threads: %?i", GlobalConfig.General.Threads);

    while (!GlobalConfig.General.Shutdown)
    {
        if (DiscordPtr)
        {
            try
            {
                if (currTime >= walletTime)
                {
                    SaveWallet();
                    walletTime = currTime + GlobalConfig.RPCManager.blockchain_save_time;
                }
            }
            catch (const Poco::Exception & exp)
            {
                PLog->error("Poco Error: %s", std::string(exp.what()));
            }
            catch (AppGeneralException & exp)
            {
                PLog->error("App Error: %s --- %s", std::string(exp.what()), exp.getGeneralError());
            }
        }
        Poco::Thread::sleep(1);
        currTime = Poco::Timestamp().epochTime();
    }

    GlobalConfig.General.Threads--;
    PLog->information("Thread Stopped! Threads: %?i", GlobalConfig.General.Threads);
}

const DiscordID& RPCManager::getBotDiscordID()
{
    return BotID;
}

std::shared_ptr<RPCProc> RPCManager::manuallyCreateRPC(const std::string& walletname, unsigned short port)
{
    std::shared_ptr<RPCProc> ret(new RPCProc(RPCMan->SpinUpNewRPC(0, port)));
    return ret;
}

RPCProc RPCManager::SpinUpNewRPC(DiscordID id, unsigned short port)
{
    RPCProc RPC_DATA;
    RPC_DATA.pid = LaunchRPC(port);
    RPC_DATA.MyRPC.open(port);
    return RPC_DATA;
}

void RPCManager::SaveWallet()
{
    Poco::Mutex::ScopedLock lock(mu);
    PLog->information("Saving blockchain...");

}

unsigned int RPCManager::LaunchRPC(unsigned short port)
{
    std::vector<std::string> args;
    args.emplace_back("--wallet-dir");
    args.emplace_back(GlobalConfig.RPC.wallet_path);
    args.emplace_back("--rpc-bind-port");
    args.push_back(Poco::format("%?i", port));
    args.emplace_back("--daemon-address");
    args.emplace_back(GlobalConfig.RPC.daemon_hostname);
    args.emplace_back("--disable-rpc-login");
    args.emplace_back("--trusted-daemon");

    if (GlobalConfig.RPC.use_test_net)
        args.emplace_back("--testnet");

    std::string launchFile;
#if _WIN32
    launchFile = GlobalConfig.RPC.filename;
#else
    launchFile = "./" + GlobalConfig.RPC.filename;
#endif
    Poco::ProcessHandle rpc_handle = Poco::Process::launch(launchFile, args, nullptr, nullptr, nullptr);
    return rpc_handle.id();
}

void RPCManager::waitForRPCToRespond(DiscordID id, const RPC & rpc)
{
    // Wait for RPC to respond to requests.
    // This is because we need to ensure open_wallet actually gets called
    // and if RPC is still loading it'll just go into the void.
    bool waitForRPC = true;

    const Poco::Timestamp timeStarted;
    while (waitForRPC)
    {
        Poco::Timespan timer(Poco::Timestamp() - timeStarted);
        try
        {
            rpc.getBlockHeight(); // Query RPC until it responds.
            waitForRPC = false;
        }
        catch (RPCConnectionError & exp)
        {
            Poco::Thread::sleep(100);
        }
        catch (const Poco::Exception & exp) // Catch network exceptions.
        {
            Poco::Thread::sleep(100);
        }
        catch (RPCGeneralError & exp) // Some other error probably no wallet file
        {
            waitForRPC = false;
        }

        if (timer.seconds() > 30)
            throw RPCGeneralError("-1", "RPC Process Killed, given up on connecting.");
    }
}
