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
#include "Account.h"
#include <string>
#include "AccountException.h"
#include "Poco/Format.h"
#include <cassert>
#include "Util.h"
#include "RPCManager.h"
#include <fstream>
#include "Poco/Thread.h"
#include "Config.h"

Account::Account() : RPCPtr(nullptr), Discord_ID(0), Balance(0), UnlockedBalance(0)
{
}

Account::Account(const Account& obj)
{
    RPCPtr = obj.RPCPtr;
    Discord_ID = obj.Discord_ID;
    Balance = obj.Balance;
    UnlockedBalance = obj.UnlockedBalance;
    MyAddress = obj.MyAddress;
}

void Account::open(DiscordID id, const RPC* ptr)
{
    Discord_ID = id;
    RPCPtr = ptr;
}

DiscordID Account::getDiscordID() const
{
    return Discord_ID;
}

std::uint64_t Account::getBalance() const
{
    return Balance;
}

std::uint64_t Account::getUnlockedBalance() const
{
    return UnlockedBalance;
}

std::uint64_t Account::getBlockHeight() const
{
    assert(RPCPtr);
    return RPCPtr->getBlockHeight();
}

const std::string & Account::getMyAddress() const
{
    return MyAddress;
}

TransferRet Account::transferMoneytoAnotherDiscordUser(std::uint64_t amount, DiscordID DIS_ID)
{
    Poco::Mutex::ScopedLock lock(mu);
    assert(RPCPtr);

    // Resync account.
    resyncAccount();

    if (amount == UnlockedBalance)
        throw InsufficientBalance("You do not have enough money for the fee, try !giveall instead");

    if (amount > UnlockedBalance)
        throw InsufficientBalance(Poco::format("You are trying to send %f while only having %f!", amount / GlobalConfig.RPC.coin_offset, UnlockedBalance / GlobalConfig.RPC.coin_offset));

    if (amount == 0)
        throw ZeroTransferAmount("You are trying to transfer a zero amount");

    if (DIS_ID == 0)
        throw GeneralAccountError("You need to specify an account to send to.");

    // Open (or create) other Discord User account to get the address
    auto recieveAccount = RPCMan->getAccount(DIS_ID);
    const std::string DiscordUserAddress = Account::getWalletAddress(DIS_ID);

    if (DIS_ID != recieveAccount.getDiscordID())
        throw GeneralAccountError("Internel Error: Trying to send money to a invalid user. v1");

    if (DiscordUserAddress != recieveAccount.getMyAddress())
        throw GeneralAccountError("Internel Error: Trying to send money to a invalid user. v2");

    if (DiscordUserAddress == MyAddress)
        throw GeneralAccountError("Don't transfer money to yourself.");

    if (DiscordUserAddress.length() != GlobalConfig.RPC.address_length)
        throw GeneralAccountError("Invalid Address.");


    auto ret = RPCPtr->tranfer(Discord_ID, amount, DiscordUserAddress);

    // Set Outgoing TX Note
    RPCPtr->setTXNote({ ret.tx_hash }, { Poco::format("%Lu", DIS_ID) });

    // Send the money
    return ret;
}

TransferRet Account::transferAllMoneytoAnotherDiscordUser(DiscordID DIS_ID)
{
    Poco::Mutex::ScopedLock lock(mu);
    assert(RPCPtr);

    // Resync account.
    resyncAccount();

    if (!UnlockedBalance)
        throw InsufficientBalance("You have an empty balance!");

    if (DIS_ID == 0)
        throw GeneralAccountError("You need to specify an account to send to.");

    // Open (or create) other Discord User account to get the address
    auto recieveAccount = RPCMan->getAccount(DIS_ID);
    const std::string DiscordUserAddress = Account::getWalletAddress(DIS_ID);

    if (DIS_ID != recieveAccount.getDiscordID())
        throw GeneralAccountError("Internel Error: Trying to send money to a invalid user. v1");

    if (DiscordUserAddress != recieveAccount.getMyAddress())
        throw GeneralAccountError("Internel Error: Trying to send money to a invalid user. v2");

    if (DiscordUserAddress == MyAddress)
        throw GeneralAccountError("Don't transfer money to yourself.");

    if (DiscordUserAddress.length() != GlobalConfig.RPC.address_length)
        throw GeneralAccountError("Invalid Address.");

    auto ret = RPCPtr->sweepAll(Discord_ID, DiscordUserAddress);

    // Set Outgoing TX Note
    RPCPtr->setTXNote({ ret.tx_hash }, { Poco::format("%Lu", DIS_ID) });

    // Send the money
    return ret;
}

TransferRet Account::transferMoneyToAddress(std::uint64_t amount, const std::string & address)
{
    Poco::Mutex::ScopedLock lock(mu);
    assert(RPCPtr);

    // Resync account.
    resyncAccount();

    if (address.length() != GlobalConfig.RPC.address_length)
        throw GeneralAccountError("Invalid Address.");

    if (amount == UnlockedBalance)
        throw InsufficientBalance("You do not have enough money for the fee, try !giveall instead");

    if (amount > UnlockedBalance)
        throw InsufficientBalance(Poco::format("You are trying to send %f while only having %f!", amount / GlobalConfig.RPC.coin_offset, UnlockedBalance / GlobalConfig.RPC.coin_offset));

    if (amount == 0)
        throw ZeroTransferAmount("You are trying to transfer a zero amount");

    if (address.empty())
        throw GeneralAccountError("You need to specify an address to send to.");

    if (address == MyAddress)
        throw GeneralAccountError("Don't transfer money to yourself.");

    auto ret = RPCPtr->tranfer(Discord_ID, amount, address);

    // Set Outgoing TX Note
    RPCPtr->setTXNote({ ret.tx_hash }, { "-1" });

    // Send the money
    return ret;
}

TransferRet Account::transferAllMoneyToAddress(const std::string& address)
{
    Poco::Mutex::ScopedLock lock(mu);
    assert(RPCPtr);

    // Resync account.
    resyncAccount();

    if (address.length() != GlobalConfig.RPC.address_length)
        throw GeneralAccountError("Invalid Address.");

    if (UnlockedBalance == 0)
        throw InsufficientBalance(Poco::format("You are trying to send all your money to an address while only having %f!", UnlockedBalance / GlobalConfig.RPC.coin_offset));

    if (address.empty())
        throw GeneralAccountError("You need to specify an address to send to.");

    if (address == MyAddress)
        throw GeneralAccountError("Don't transfer money to yourself.");

    auto ret = RPCPtr->sweepAll(Discord_ID, address);

    // Set Outgoing TX Note
    RPCPtr->setTXNote({ ret.tx_hash }, { "-1" });

    // Send the money
    return ret;
}

TransferList Account::getTransactions()
{
    assert(RPCPtr);
    auto ret = RPCMan->getTransfers(Discord_ID);

    std::vector<std::string> TXs;

    for (const auto & tx : ret.tx_out)
        TXs.emplace_back(tx.tx_hash);

    if (!TXs.empty())
    {
        auto notes = RPCPtr->getTXNote(TXs);

        auto note_it = notes.begin();

        // Filter Transactions
        for (auto & tx : ret.tx_out)
        {
            if (*note_it != "-1")
            {
                if (!Poco::NumberParser::tryParseUnsigned64(*note_it, tx.payment_id))
                {
                    tx.payment_id = 0;
                }
            }
            else tx.payment_id = 0;
            note_it++;
        }
    }
    return ret;
}

void Account::resyncAccount()
{
    Poco::Mutex::ScopedLock lock(mu);
    assert(RPCPtr);
    RPCPtr->rescanSpent();
    Poco::Thread::sleep(500); // Sleep for a bit so RPC can catch up.
    const auto Bal = RPCPtr->getBalance();
    Balance = Bal.Balance;
    UnlockedBalance = Bal.UnlockedBalance;
    MyAddress = RPCPtr->getAddress();
}

Account& Account::operator=(const Account& rhs)
{
    RPCPtr = rhs.RPCPtr;
    Discord_ID = rhs.Discord_ID;
    Balance = rhs.Balance;
    UnlockedBalance = rhs.UnlockedBalance;
    MyAddress = rhs.MyAddress;
    return *this;
}

const std::string Account::getWalletAddress(DiscordID Discord_ID)
{
    static unsigned int tmp_rpc_counter = 1; // Lottery is on port starting - 1.
                                             // when it enters the code below it'll be 2. 
    const std::string & walletStr = Util::getWalletStrFromIID(Discord_ID);

    if (!Util::doesWalletExist(GlobalConfig.RPC.wallet_path + walletStr))
    {
        // All this code is a hack because the Monero RPC documentation didn't mention that
        // when you create a wallet the RPC opens that wallet automatically so theres a design flaw.
        tmp_rpc_counter++;
        std::shared_ptr<RPCProc> rpcptr;
        try
        {
            rpcptr = RPCManager::manuallyCreateRPC(walletStr, GlobalConfig.RPCManager.starting_port_number - tmp_rpc_counter);
            rpcptr->MyRPC.createWallet(walletStr);
            try
            {
                rpcptr->MyRPC.stopWallet();
            }
            catch (...)
            {
                Poco::Process::kill(rpcptr->pid);
            }
            RPCMan->getAccount(Discord_ID); // Add account to the RPCManager. 
        }
        catch (...)
        {
            if (rpcptr && rpcptr->pid)
                Poco::Process::kill(rpcptr->pid);
            throw RPCGeneralError("-1", "Could not create wallet!");
        }
        tmp_rpc_counter--;
    }

    const auto addressStr = GlobalConfig.RPC.wallet_path + walletStr + ".address.txt";

    std::ifstream infile(addressStr);
    assert(infile.is_open());
    return { std::istreambuf_iterator<char>(infile), std::istreambuf_iterator<char>() };
}
