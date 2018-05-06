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

RPCManager      RPCMan;

RPCManager::RPCManager() : currPortNum(STARTING_PORT_NUMBER), DiscordPtr(nullptr)
{
    
}

RPCManager::~RPCManager()
{
    // Save blockchain on exit.
    try
    {
        save();
        SaveWallets();
    }
    catch (...)
    {
        // We need to shutdown RPCs and we can't crash else they don't get shutdown.
    }
}

void RPCManager::setBotUser(DiscordID id)
{
    BotID = id;
}

void RPCManager::setDiscordPtr(ITNS_TIPBOT* ptr)
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
        if (RPCMap.size() <= MAX_RPC_LIMIT)
            RPCMap[id] = SpinUpNewRPC(id);
        else
            RPCMap[id] = FindOldestRPC();

        // Setup Account
        RPCMap[id].MyAccount.open(id, &RPCMap[id].MyRPC);

        // Wait for RPC to respond
        waitForRPCToRespond(id, RPCMap[id].MyRPC);

        // Open Wallet
        RPCMap[id].MyRPC.openWallet(Util::getWalletStrFromIID(id));

        // Get transactions
        RPCMap[id].Transactions = RPCMap[id].MyRPC.getTransfers();
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
    const Poco::Timestamp timeStarted;
    while (true)
    {
        Poco::Timespan timer(Poco::Timestamp() - timeStarted);
        if (DiscordPtr)
        {
            try
            {
                if ((timer.seconds() % SEARCH_FOR_NEW_TRANSACTIONS_TIME) == 0)
                {

                    processNewTransactions();
                }

                if ((timer.minutes() > 0) && (timer.seconds() == 0) && (timer.minutes() % BLOCKCHAIN_SAVE_TIME) == 0)
                {
                    SaveWallets();
                }

                if ((timer.seconds() % RPC_WALLETS_SAVE_TIME) == 0)
                {
                    save();
                }
            }
            catch (const Poco::Exception & exp)
            {
                std::cerr << "Poco Error: " << exp.what() << "\n";
            }
            catch (AppGeneralException & exp)
            {
                std::cerr << "App Error: " << exp.what() << "\n";
            }
            catch (const SleepyDiscord::ErrorCode & exp)
            {
                std::cerr << Poco::format("Discord Error Code: --- %d\n", exp);
            }
        }
        Poco::Thread::sleep(1000);
    }
}

void RPCManager::processNewTransactions()
{
    Poco::Mutex::ScopedLock lock(mu);
    std::cout << "Searching for new transactions...\n";
    std::vector<struct TransferItem> diff;
    TransferList newTransactions;

    for (auto & account : this->RPCMap)
    {
        if (account.first != BotID)
        {
            newTransactions = account.second.MyRPC.getTransfers();

            std::set_difference(newTransactions.tx_in.begin(), newTransactions.tx_in.end(), account.second.Transactions.tx_in.begin(), account.second.Transactions.tx_in.end(),
                std::inserter(diff, diff.begin()));

            if (!diff.empty())
            {
                try
                {
                    for (auto newTx : diff)
                    {
                        DiscordPtr->sendMessage(DiscordPtr->getDiscordDMChannel(account.first), Poco::format("You've recieved money! %0.8f ITNS :money_with_wings:", newTx.amount / ITNS_OFFSET));
                        std::cout << Poco::format("User %Lu recived %0.8f ITNS\n", account.first, newTx.amount / ITNS_OFFSET);
                    }

                    account.second.Transactions = newTransactions;

                    diff.clear();
                }
                catch (const Poco::Exception & exp)
                {
                    diff.clear();
                }
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
    return RPCMan.getRPC(RPCMan.getBotDiscordID());
}

Account & RPCManager::getGlobalBotAccount()
{
    return RPCMan.getAccount(RPCMan.getBotDiscordID());
}

const DiscordID& RPCManager::getBotDiscordID()
{
    return BotID;
}

std::shared_ptr<RPCProc> RPCManager::manuallyCreateRPC(const std::string& walletname, unsigned short port)
{
    std::shared_ptr<RPCProc> ret(new RPCProc(RPCMan.SpinUpNewRPC(0, port)));

    // Setup Account
    ret->MyAccount.open(RPCMan.getBotDiscordID(), &ret->MyRPC);

    // Wait for RPC to respond
    RPCMan.waitForRPCToRespond(0, ret->MyRPC);

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
        std::cout << "Saving wallet data to disk...\n";
        {
            cereal::JSONOutputArchive ar(out);
            ar(CEREAL_NVP(currPortNum), CEREAL_NVP(RPCMap));
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
        std::cout << "Loading wallet files and spinning up RPCs..\n";
        std::ifstream in(RPC_DATABASE_FILENAME);
        if (in.is_open())
        {
            {
                cereal::JSONInputArchive ar(in);
                ar(CEREAL_NVP(currPortNum), CEREAL_NVP(RPCMap));
            }
            in.close();
            ReloadSavedRPCs();
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

RPCProc& RPCManager::FindOldestRPC()
{
    auto it = std::min_element(RPCMap.begin(), RPCMap.end(),
        [](decltype(RPCMap)::value_type& l, decltype(RPCMap)::value_type& r) -> bool { return l.second.timestamp < r.second.timestamp; });

    return it->second;
}

void RPCManager::SaveWallets()
{
    Poco::Mutex::ScopedLock lock(mu);
    std::cout << "Saving blockchain...\n";

    // Save blockchain on exit.
    for (auto account : this->RPCMap)
        account.second.MyRPC.store();
}

void RPCManager::ReloadSavedRPCs()
{
    for (auto & wallets : RPCMap)
    {
        try
        {
            // Launch RPC
            wallets.second.pid = LaunchRPC(wallets.second.MyRPC.getPort());

            // Setup Accounts
            wallets.second.MyAccount.open(wallets.first, &wallets.second.MyRPC);

            // Wait for RPC to respond
            waitForRPCToRespond(wallets.first, wallets.second.MyRPC);

            // Open Wallet
            wallets.second.MyRPC.openWallet(Util::getWalletStrFromIID(wallets.first));

            // Get transactions
            wallets.second.Transactions = wallets.second.MyRPC.getTransfers();
        }
        catch (const Poco::Exception & exp)
        {
            std::cerr << "Poco Error: " << exp.what();
        }
        catch (AppGeneralException & exp)
        {
            std::cerr << "App Error: " << exp.what();
        }
    }
}

unsigned int RPCManager::LaunchRPC(unsigned short port)
{
    std::vector<std::string> args;
    args.emplace_back("--wallet-dir");
    args.emplace_back(WALLET_PATH);
    args.emplace_back("--rpc-bind-port");
    args.push_back(Poco::format("%?i", port));
    args.emplace_back("--daemon-address");
    args.emplace_back(DAEMON_ADDRESS);
    args.emplace_back("--disable-rpc-login");
    args.emplace_back("--trusted-daemon");

    Poco::ProcessHandle rpc_handle = Poco::Process::launch(RPC_FILENAME, args, nullptr, nullptr, nullptr);
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

void RPCManager::rescanAll()
{
    for (auto & wallet : RPCMap)
        wallet.second.MyRPC.rescanSpent();
}
