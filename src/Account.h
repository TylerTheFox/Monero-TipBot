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

#define VOID_WALLET			"VOID_WALLET" // This is opened when the account is closed.

class Account
{
public:
	Account();
	
	void					open(std::uint64_t Discord_ID);

	std::uint64_t			getBalance() const;
	std::uint64_t			getUnlockedBalance() const;
	const std::string &		getMyAddress() const;

	TransferRet				transferMoneytoAnotherDiscordUser(std::uint64_t amount, std::uint64_t Discord_ID) const;
	TransferRet				transferAllMoneytoAnotherDiscordUser(std::uint64_t Discord_ID) const;
	TransferRet				transferMoneyToAddress(std::uint64_t amount, const std::string & address) const;
	TransferRet				transferAllMoneyToAddress(const std::string & address) const;

	TransferList			getTransactions();
private:
	std::uint64_t			Discord_ID;
	std::uint64_t			Balance;
	std::uint64_t			UnlockedBalance;
	std::string				MyAddress;

	void					resyncAccount();
};