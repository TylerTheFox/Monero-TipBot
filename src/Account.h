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
#include "RPC.h"
#include <string>
#include "types.h"

class Account
{
public:
    Account();
    Account(const Account & obj);

    void                        open(DiscordID id, const RPC * ptr);

    DiscordID                   getDiscordID() const;
    std::uint64_t               getBalance() const;
    std::uint64_t               getUnlockedBalance() const;
    std::uint64_t               getBlockHeight() const;
    const std::string &         getMyAddress() const;

    TransferRet                 transferMoneytoAnotherDiscordUser(std::uint64_t amount, DiscordID Discord_ID);
    TransferRet                 transferAllMoneytoAnotherDiscordUser(DiscordID Discord_ID);
    TransferRet                 transferMoneyToAddress(std::uint64_t amount, const std::string & address);
    TransferRet                 transferAllMoneyToAddress(const std::string & address);

    TransferList                getTransactions();
    static const std::string    getWalletAddress(DiscordID Discord_ID);
    void                        resyncAccount();

    Account&                    operator=(const Account &rhs);

private:
    const RPC*                  RPCPtr;
    DiscordID                   Discord_ID;
    std::uint64_t               Balance;
    std::uint64_t               UnlockedBalance;
    std::string                 MyAddress;
};