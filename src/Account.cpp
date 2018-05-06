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
    assert(RPCPtr);

    // Resync account.
    resyncAccount();

    if (amount == UnlockedBalance)
        throw InsufficientBalance("You do not have enough money for the fee, try !giveall instead");

    if (amount > UnlockedBalance)
        throw InsufficientBalance(Poco::format("You are trying to send %f while only having %f!", amount / ITNS_OFFSET, UnlockedBalance / ITNS_OFFSET));

    if (amount == 0)
        throw ZeroTransferAmount("You are trying to transfer a zero amount");

    if (DIS_ID == 0)
        throw GeneralAccountError("You need to specify an account to send to.");

    // Open (or create) other Discord User account to get the address
    auto recieveAccount = RPCMan.getAccount(DIS_ID);
    const std::string DiscordUserAddress = recieveAccount.getMyAddress();

    if (DiscordUserAddress == MyAddress)
        throw GeneralAccountError("Don't transfer money to yourself.");

    // Send the money
    return RPCPtr->tranfer(Discord_ID, amount, DiscordUserAddress);
}

TransferRet Account::transferAllMoneytoAnotherDiscordUser(DiscordID DIS_ID)
{
    assert(RPCPtr);

    // Resync account.
    resyncAccount();

    if (!UnlockedBalance)
        throw InsufficientBalance("You have an empty balance!");

    if (DIS_ID == 0)
        throw GeneralAccountError("You need to specify an account to send to.");

    // Open (or create) other Discord User account to get the address
    auto recieveAccount = RPCMan.getAccount(DIS_ID);
    const std::string DiscordUserAddress = recieveAccount.getMyAddress();

    if (DiscordUserAddress == MyAddress)
        throw GeneralAccountError("Don't transfer money to yourself.");

    // Send the money
    return RPCPtr->sweepAll(Discord_ID, DiscordUserAddress);
}

TransferRet Account::transferMoneyToAddress(std::uint64_t amount, const std::string & address)
{
    assert(RPCPtr);

    // Resync account.
    resyncAccount();

    if (amount == UnlockedBalance)
        throw InsufficientBalance("You do not have enough money for the fee, try !giveall instead");

    if (amount > UnlockedBalance)
        throw InsufficientBalance(Poco::format("You are trying to send %f while only having %f!", amount / ITNS_OFFSET, UnlockedBalance / ITNS_OFFSET));

    if (amount == 0)
        throw ZeroTransferAmount("You are trying to transfer a zero amount");

    if (address.empty())
        throw GeneralAccountError("You need to specify an address to send to.");

    if (address == MyAddress)
        throw GeneralAccountError("Don't transfer money to yourself.");

    // Send the money
    return RPCPtr->tranfer(Discord_ID, amount, address);
}

TransferRet Account::transferAllMoneyToAddress(const std::string& address)
{
    assert(RPCPtr);

    // Resync account.
    resyncAccount();

    if (UnlockedBalance == 0)
        throw InsufficientBalance(Poco::format("You are trying to send all your money to an address while only having %f!", UnlockedBalance / ITNS_OFFSET));

    if (address.empty())
        throw GeneralAccountError("You need to specify an address to send to.");

    if (address == MyAddress)
        throw GeneralAccountError("Don't transfer money to yourself.");

    // Send the money
    return RPCPtr->sweepAll(Discord_ID, address);
}

TransferList Account::getTransactions()
{
    assert(RPCPtr);
    return RPCPtr->getTransfers();
}

void Account::resyncAccount()
{
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
    const std::string & walletStr = Util::getWalletStrFromIID(Discord_ID);

    if (!Util::doesWalletExist(WALLET_PATH + walletStr))
        assert(RPCManager::getGlobalBotRPC().createWallet(walletStr));

    const auto addressStr = WALLET_PATH + walletStr + ".address.txt";

    std::ifstream infile(addressStr);
    assert(infile.is_open());
    return { std::istreambuf_iterator<char>(infile), std::istreambuf_iterator<char>() };
}
