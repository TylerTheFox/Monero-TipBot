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

#define DISCORD_WALLET_MASK "Discord-User-%Lu"
#define VOID_WALLET			"VOID_WALLET" // This is opened when the account is closed.

class Account
{
public:
	Account() = delete;
	Account(unsigned long long Discord_ID);

	unsigned long long		getBalance() const;
	unsigned long long		getUnlockedBalance() const;
	const std::string &		getMyAddress() const;

	TransferRet				transferMoneytoAnotherDiscordUser(unsigned long long amount, unsigned long long Discord_ID) const;
	TransferRet				transferAllMoneytoAnotherDiscordUser(unsigned long long Discord_ID) const;
	TransferRet				transferMoneyToAddress(unsigned long long amount, const std::string & address) const;
	TransferRet				transferAllMoneyToAddress(const std::string & address) const;

	TransferList			getTransactions();
private:
	static bool				FirstTime;
	RPC						RPCServ;
	unsigned long long		Discord_ID;
	unsigned long long		Balance{};
	unsigned long long		UnlockedBalance{};
	std::string				MyAddress;

	void					resyncAccount();
};