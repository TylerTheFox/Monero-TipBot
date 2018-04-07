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

bool Account::FirstTime = true;

Account::Account(unsigned long long DIS_ID) : Discord_ID(DIS_ID)
{
	// This is require to save any previous wallets blockchain.
	if (!FirstTime)
	{
		RPCServ.store();
	}

	const std::string Wallet_Name = Poco::format(DISCORD_WALLET_MASK, DIS_ID);
	assert(RPCServ.openWallet(Wallet_Name));
	resyncAccount();

	FirstTime = false;
}

unsigned long long Account::getBalance() const
{
	return Balance;
}

unsigned long long Account::getUnlockedBalance() const
{
	return UnlockedBalance;
}

const std::string & Account::getMyAddress() const
{
	return MyAddress;
}

TransferRet Account::transferMoneytoAnotherDiscordUser(unsigned long long amount, unsigned long long DIS_ID) const
{
	if (amount > Balance)
		throw InsufficientBalance(Poco::format("You are trying to send %f while only having %f!", amount / ITNS_OFFSET, Balance / ITNS_OFFSET));

	if (amount == 0)
		throw ZeroTransferAmount("You are trying to transfer a zero amount");

	if (DIS_ID == 0)
		throw GeneralAccountError("You need to specify an account to send to.");

	// Open (or create) other Discord User account to get the address
	std::string Wallet_Name = Poco::format(DISCORD_WALLET_MASK, DIS_ID);
	assert(RPCServ.openWallet(Wallet_Name));
	const std::string DiscordUserAddress = RPCServ.getAddress();

	// Now they we got the address reopen my account so we can send the money.
	Wallet_Name = Poco::format(DISCORD_WALLET_MASK, Discord_ID);
	assert(RPCServ.openWallet(Wallet_Name));

	// Send the money
	return RPCServ.tranfer(Discord_ID, amount, DiscordUserAddress);
}

TransferRet Account::transferAllMoneytoAnotherDiscordUser(unsigned long long DIS_ID) const
{
	if (!Balance)
		throw InsufficientBalance("You have an empty balance!");

	if (DIS_ID == 0)
		throw GeneralAccountError("You need to specify an account to send to.");

	// Open (or create) other Discord User account to get the address
	std::string Wallet_Name = Poco::format(DISCORD_WALLET_MASK, DIS_ID);
	assert(RPCServ.openWallet(Wallet_Name));
	const std::string DiscordUserAddress = RPCServ.getAddress();

	// Now they we got the address reopen my account so we can send the money.
	Wallet_Name = Poco::format(DISCORD_WALLET_MASK, Discord_ID);
	assert(RPCServ.openWallet(Wallet_Name));

	// Send the money
	return RPCServ.sweepAll(Discord_ID, DiscordUserAddress);
}

TransferRet Account::transferMoneyToAddress(unsigned long long amount, const std::string & address) const
{	
	if (amount > Balance)
		throw InsufficientBalance(Poco::format("You are trying to send %f while only having %f!", amount / ITNS_OFFSET, Balance / ITNS_OFFSET));

	if (amount == 0)
		throw ZeroTransferAmount("You are trying to transfer a zero amount");

	if (address.empty())
		throw GeneralAccountError("You need to specify an address to send to.");

	// Send the money
	return RPCServ.tranfer(Discord_ID, amount, address);
}

TransferRet Account::transferAllMoneyToAddress(const std::string& address) const
{
	if (Balance == 0)
		throw InsufficientBalance(Poco::format("You are trying to send all your money to an address while only having %f!", Balance / ITNS_OFFSET));

	if (address.empty())
		throw GeneralAccountError("You need to specify an address to send to.");

	// Send the money
	return RPCServ.sweepAll(Discord_ID, address);
}

TransferList Account::getTransactions()
{
	return RPCServ.getTransfers();
}

void Account::resyncAccount()
{
	const auto Bal = RPCServ.getBalance();
	Balance = Bal.Balance;
	UnlockedBalance = Bal.UnlockedBalance;
	MyAddress = RPCServ.getAddress();
}
