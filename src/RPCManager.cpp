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
#include "Discord.h"
#include "RPCException.h"
#include "Poco/File.h"
#include "cereal/types/map.hpp"
#include "cereal/types/set.hpp"
#include <fstream>
#include "AccountException.h"
#include "Poco/ScopedLock.h"
#include "Config.h"
std::unique_ptr<RPCManager>      RPCMan;

RPCManager::RPCManager() : currPortNum(GlobalConfig.RPCManager.starting_port_number), DiscordPtr(nullptr), BotID(0)
{
    PLog = &Poco::Logger::get("RPCManager");
}

RPCManager::~RPCManager()
{
    save();
    SaveWallets();

    // Kill all running RPCs
    for (auto account : this->RPCMap)
    {
        try
        {
            account.second.MyRPC.stopWallet();
        }
        catch (...)
        {
            try
            {
                Poco::Process::kill(account.second.pid);
            }
            catch (...)
            {
                // Ignore.
            }
        }
    }
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
        PLog->error("App Error: %s", exp.getGeneralError());
    }
}

void RPCManager::setDiscordPtr(TIPBOT* ptr)
{
    DiscordPtr = ptr;
}

bool RPCManager::isRPCRunning(DiscordID id)
{
    return (RPCMap.count(id) == 1);
}

time_t RPCManager::getTimeStarted(DiscordID id)
{
    return RPCMap[id].timestamp.epochTime();
}

Account& RPCManager::getAccount(DiscordID id)
{
    Poco::Mutex::ScopedLock lock(mu);

    if (RPCMap.count(id) == 0)
    {
        try
        {
            if (RPCMap.size() < GlobalConfig.RPCManager.max_rpc_limit)
                RPCMap[id] = SpinUpNewRPC(id);
            else
            {
                RPCProc oldAcc = FindOldestRPC(); // Deep copy
                auto oldAccID = oldAcc.MyAccount.getDiscordID();
                RPCMap.erase(oldAccID); // Ensure we destroy the old account --- SECURITY BUG FIX ---
                RPCMap[id] = oldAcc;
            }

            // Setup Account
            RPCMap[id].MyAccount.open(id, &RPCMap[id].MyRPC);

            // Wait for RPC to respond
            waitForRPCToRespond(id, RPCMap[id].MyRPC);

            // Open Wallet
            RPCMap[id].MyRPC.openWallet(Util::getWalletStrFromIID(id));

            // Get transactions
            RPCMap[id].Transactions = RPCMap[id].MyRPC.getTransfers();
        }
        catch (AppGeneralException & exp)
        {
            // Shutdown running RPCs
            if (RPCMap[id].pid)
                Poco::Process::kill(RPCMap[id].pid);

            // Erase RPC
            RPCMap.erase(id);

            // Continue Error throwing.
            throw RPCGeneralError("-1", exp.what());
        }
    }

    // Ensure we are the correct account owners.
    if (Account::getWalletAddress(id) != RPCMap[id].MyRPC.getAddress())
    {
        // ABORT ABORT ABORT!
        RPCMap.erase(id);
        throw RPCGeneralError("-1", "You do not own this account!");
    }

    // Update timestamp
    RPCMap[id].timestamp = Poco::Timestamp();

    // Account Resync
    RPCMap[id].MyAccount.resyncAccount();

    return RPCMap[id].MyAccount;
}

const TransferList RPCManager::getTransfers(DiscordID id)
{
    if (RPCMap.count(id))
        return RPCMap[id].Transactions;
    return {};
}

void RPCManager::run()
{
    time_t  currTime = Poco::Timestamp().epochTime();
    time_t  transactionTime = currTime + GlobalConfig.RPCManager.search_for_new_transactions_time,
        walletTime = currTime + GlobalConfig.RPCManager.blockchain_save_time,
        saveTime = currTime + GlobalConfig.RPCManager.wallets_save_time,
        walletWatchDog = currTime + GlobalConfig.RPCManager.wallet_watchdog_time;

    GlobalConfig.General.Threads++;

    while (!GlobalConfig.General.Shutdown)
    {
        if (DiscordPtr)
        {
            try
            {
                if (currTime >= walletWatchDog)
                {
                    watchDog();
                    walletWatchDog = currTime + GlobalConfig.RPCManager.wallet_watchdog_time;
                }

                if (currTime >= transactionTime)
                {
                    processNewTransactions();
                    transactionTime = currTime + GlobalConfig.RPCManager.search_for_new_transactions_time;
                }

                if (currTime >= walletTime)
                {
                    SaveWallets();
                    walletTime = currTime + GlobalConfig.RPCManager.blockchain_save_time;
                }

                if (currTime >= saveTime)
                {
                    save();
                    saveTime = currTime + GlobalConfig.RPCManager.wallets_save_time;
                }
            }
            catch (const Poco::Exception & exp)
            {
                PLog->error("Poco Error: %s", std::string(exp.what()));
            }
            catch (AppGeneralException & exp)
            {
                PLog->error("App Error: %s", std::string(exp.what()));
            }
            catch (const SleepyDiscord::ErrorCode & exp)
            {
                PLog->error(Poco::format("Discord Error Code: --- %d\n", exp));
            }
        }
        Poco::Thread::sleep(1);
        currTime = Poco::Timestamp().epochTime();
    }

    GlobalConfig.General.Threads--;
}

void RPCManager::processNewTransactions()
{
    Poco::Mutex::ScopedLock lock(mu);
    PLog->information("Searching for new transactions...");
    std::vector<struct TransferItem> diff;
    TransferList newTransactions;

    for (auto & account : this->RPCMap)
    {
        if (account.first != BotID)
        {
            newTransactions = account.second.MyRPC.getTransfers();

            std::set_difference(newTransactions.tx_in.begin(), newTransactions.tx_in.end(), account.second.Transactions.tx_in.begin(), account.second.Transactions.tx_in.end(),
                std::inserter(diff, diff.begin()));

            account.second.Transactions = newTransactions;

            if (!diff.empty())
            {
                try
                {
                    try
                    {
                        for (auto newTx : diff)
                        {
                            DiscordPtr->sendMessage(DiscordPtr->getDiscordDMChannel(account.first), Poco::format("You've recieved money! %0.8f %s :money_with_wings:", newTx.amount / GlobalConfig.RPC.coin_offset, GlobalConfig.RPC.coin_abbv));
                            PLog->information(Poco::format("User %Lu recived %0.8f %s\n", account.first, newTx.amount / GlobalConfig.RPC.coin_offset, GlobalConfig.RPC.coin_abbv));
                        }
                    }
                    catch (const SleepyDiscord::ErrorCode & exp)
                    {
                        PLog->error("Error while posting transactions for user: %?i Error code: %?i", account.first, exp);
                    }

                    diff.clear();
                    account.second.MyRPC.store();
                }
                catch (const Poco::Exception & exp)
                {
                    diff.clear();
                }
            }
        }
    }
}

void RPCManager::watchDog()
{
    Poco::Mutex::ScopedLock lock(mu);
    for (auto rpc : RPCMap)
    {
        try
        {
            if (Poco::Process::isRunning(rpc.second.pid))
            {
                rpc.second.MyRPC.getBlockHeight();
            }
            else
            {
                // RPC is not running, kill it.
                PLog->error("User %?i's RPC is not running, RPC restarted!", rpc.first);
                restartWallet(rpc.first);
            }
            rpc.second.RPCFail = 0;
        }
        catch (...)
        {
            PLog->error("User %?i's RPC is not responding/erroring for %?i out of %?i attempts!", rpc.first, rpc.second.RPCFail, GlobalConfig.RPCManager.error_giveup);
            rpc.second.RPCFail++;
            if (rpc.second.RPCFail > GlobalConfig.RPCManager.error_giveup)
            {
                // RPC not responding, kill it.
                PLog->error("User %?i's RPC is not responding, RPC restarted!", rpc.first);
                restartWallet(rpc.first);
                rpc.second.RPCFail = 0;
            }
        }
    }
}

const RPC& RPCManager::getRPC(DiscordID id)
{
    if (RPCMap.count(id))
        return RPCMap[id].MyRPC;
    throw RPCGeneralError("-1", "RPC does not exist!");
}

const RPC & RPCManager::getGlobalBotRPC()
{
    return RPCMan->getRPC(RPCMan->getBotDiscordID());
}

Account & RPCManager::getGlobalBotAccount()
{
    return RPCMan->getAccount(RPCMan->getBotDiscordID());
}

const DiscordID& RPCManager::getBotDiscordID()
{
    return BotID;
}

std::shared_ptr<RPCProc> RPCManager::manuallyCreateRPC(const std::string& walletname, unsigned short port)
{
    std::shared_ptr<RPCProc> ret(new RPCProc(RPCMan->SpinUpNewRPC(0, port)));

    // Setup Account
    ret->MyAccount.open(RPCMan->getBotDiscordID(), &ret->MyRPC);

    // Wait for RPC to respond
    RPCMan->waitForRPCToRespond(0, ret->MyRPC);

    // Open Wallet
    ret->MyRPC.openWallet(walletname);

    // Get transactions
    ret->Transactions = ret->MyRPC.getTransfers();

    return ret;
}

void RPCManager::save()
{
    Poco::Mutex::ScopedLock lock(mu);
    std::ofstream out(RPC_DATABASE_FILENAME, std::ios::trunc);
    if (out.is_open())
    {
        PLog->information("Saving wallet data to disk...");
        {
            cereal::JSONOutputArchive ar(out);
            ar(
                ::cereal::make_nvp("major", GlobalConfig.About.major),
                ::cereal::make_nvp("minor", GlobalConfig.About.minor),
                CEREAL_NVP(RPCMap)
            );
        }
        out.close();
    }
}

void RPCManager::load()
{
    Poco::Mutex::ScopedLock lock(mu);
    Poco::File RPCFile(RPC_DATABASE_FILENAME);
    if (RPCFile.exists())
    {
        PLog->information("Loading wallet files and spinning up RPCs..");
        std::ifstream in(RPC_DATABASE_FILENAME);
        if (in.is_open())
        {
            {
                cereal::JSONInputArchive ar(in);

                if (GlobalConfig.About.major > 2 || GlobalConfig.About.major >= 2 && GlobalConfig.About.minor > 0)
                {
                    ar(
                        ::cereal::make_nvp("major", GlobalConfig.About.major),
                        ::cereal::make_nvp("minor", GlobalConfig.About.minor)
                    );
                }

                ar(
                    CEREAL_NVP(RPCMap)
                );
            }
            ReloadSavedRPCs();
            in.close();
        }
    }
}

RPCProc RPCManager::SpinUpNewRPC(DiscordID id, unsigned short port)
{
    RPCProc RPC_DATA;

    const auto & portNum = port ? port : currPortNum;

    RPC_DATA.pid = LaunchRPC(portNum);
    RPC_DATA.MyAccount.open(id, &RPC_DATA.MyRPC);
    RPC_DATA.MyRPC.open(portNum);

    if (port == 0)
        currPortNum++;

    return RPC_DATA;
}

void RPCManager::SpinDownRPC(DiscordID id)
{
    RPCMap[id].MyRPC.stopWallet();
    RPCMap.erase(id);
}

RPCProc & RPCManager::FindOldestRPC()
{
    auto it = std::min_element(RPCMap.begin(), RPCMap.end(),
        [](decltype(RPCMap)::value_type& l, decltype(RPCMap)::value_type& r) -> bool { return l.second.timestamp < r.second.timestamp; });

    return it->second;
}

void RPCManager::SaveWallets()
{
    Poco::Mutex::ScopedLock lock(mu);
    PLog->information("Saving blockchain...");

    // Save blockchain on exit.
    for (auto account : this->RPCMap)
    {
        try
        {
            account.second.MyRPC.store();
        }
        catch (...)
        {
            PLog->error("Error cannot save %?i's wallet!", account.first);
        }
    }
}

void RPCManager::ReloadSavedRPCs()
{
    for (auto & wallets : RPCMap)
    {
        try
        {
            // Launch RPC
            wallets.second.pid = LaunchRPC(currPortNum);

            // Set Port number
            wallets.second.MyRPC.open(currPortNum);

            // Setup Accounts
            wallets.second.MyAccount.open(wallets.first, &wallets.second.MyRPC);

            // Wait for RPC to respond
            waitForRPCToRespond(wallets.first, wallets.second.MyRPC);

            // Open Wallet
            wallets.second.MyRPC.openWallet(Util::getWalletStrFromIID(wallets.first));

            // Get transactions
            wallets.second.Transactions = wallets.second.MyRPC.getTransfers();

            currPortNum++;
        }
        catch (const Poco::Exception & exp)
        {
            PLog->error("Poco Error: %s", std::string(exp.what()));
        }
        catch (AppGeneralException & exp)
        {
            PLog->error("App Error: %s", std::string(exp.what()));
        }
    }
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
        {
            // Give up
            if (id)
            {
                Poco::Process::kill(RPCMap[id].pid);
                RPCMap.erase(id);
            }
            throw RPCGeneralError("-1", "RPC Process Killed, given up on connecting.");
        }
    }
}

std::uint64_t RPCManager::getTotalBalance()
{
    uint64_t ret = 0;
    for (auto & wallet : RPCMap)
    {
        wallet.second.MyAccount.resyncAccount();
        ret += wallet.second.MyAccount.getBalance();
    }
    return ret;
}

std::uint64_t RPCManager::getTotalUnlockedBalance()
{
    uint64_t ret = 0;
    for (auto & wallet : RPCMap)
    {
        wallet.second.MyAccount.resyncAccount();
        ret += wallet.second.MyAccount.getUnlockedBalance();
    }
    return ret;
}

void RPCManager::restartWallet(DiscordID id)
{
    Poco::Mutex::ScopedLock lock(mu);
    if (RPCMap.count(id))
    {
        PLog->information("Restarting User: %?i's RPC ", id);
        try
        {
            Poco::Process::kill(RPCMap[id].pid);
        }
        catch (const Poco::Exception & exp)
        {
            PLog->error("Error killing RPC: %s", std::string(exp.what()));
        }

        // Launch RPC
        RPCMap[id].pid = LaunchRPC(RPCMap[id].MyRPC.getPort());

        // Setup Accounts
        RPCMap[id].MyAccount.open(id, &RPCMap[id].MyRPC);

        // Wait for RPC to respond
        waitForRPCToRespond(id, RPCMap[id].MyRPC);

        // Open Wallet
        RPCMap[id].MyRPC.openWallet(Util::getWalletStrFromIID(id));

        // Get transactions
        RPCMap[id].Transactions = RPCMap[id].MyRPC.getTransfers();

        PLog->information("User: %?i's RPC restarted successfully!", id);
    }
}

void RPCManager::rescanAll()
{
    for (auto & wallet : RPCMap)
        wallet.second.MyRPC.rescanSpent();
}

void RPCManager::saveallWallets()
{
    SaveWallets();
}

std::string RPCManager::status()
{
    std::stringstream ss;
    std::uint64_t totalUnlocked = 0, TotalLocked = 0;

    for (auto account : RPCMap)
    {
        totalUnlocked += account.second.MyAccount.getUnlockedBalance();
        TotalLocked += account.second.MyAccount.getBalance() - account.second.MyAccount.getUnlockedBalance();
    }

    ss << "There is currently " << RPCMap.size() << " RPC's running\\n";
    ss << "There is a total of " << totalUnlocked / GlobalConfig.RPC.coin_offset << " Unlocked " << GlobalConfig.RPC.coin_abbv << " and a total of " << TotalLocked / GlobalConfig.RPC.coin_offset << " " << GlobalConfig.RPC.coin_abbv << " locked\\n";
    ss << "The current port number is " << currPortNum << "\\n";
    ss << "There is currently " << GlobalConfig.General.Threads << " Threads running \\n";
    return ss.str();
}