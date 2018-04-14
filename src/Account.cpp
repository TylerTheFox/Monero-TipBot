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

const std::string & Account::getMyAddress() const
{
	return MyAddress;
}

TransferRet Account::transferMoneytoAnotherDiscordUser(std::uint64_t amount, DiscordID DIS_ID) const
{
	assert(RPCPtr);

	if (amount > Balance)
		throw InsufficientBalance(Poco::format("You are trying to send %f while only having %f!", amount / ITNS_OFFSET, Balance / ITNS_OFFSET));

	if (amount == 0)
		throw ZeroTransferAmount("You are trying to transfer a zero amount");

	if (DIS_ID == 0)
		throw GeneralAccountError("You need to specify an account to send to.");

	// Open (or create) other Discord User account to get the address
	std::string Wallet_Name = Poco::format(DISCORD_WALLET_MASK, DIS_ID);
	assert(RPCPtr->openWallet(Wallet_Name));
	const std::string DiscordUserAddress = RPCPtr->getAddress();

	if (DiscordUserAddress == MyAddress)
		throw GeneralAccountError("Don't transfer money to yourself.");

	// Now they we got the address reopen my account so we can send the money.
	Wallet_Name = Poco::format(DISCORD_WALLET_MASK, Discord_ID);
	assert(RPCPtr->openWallet(Wallet_Name));

	// Send the money
	return RPCPtr->tranfer(Discord_ID, amount, DiscordUserAddress);
}

TransferRet Account::transferAllMoneytoAnotherDiscordUser(DiscordID DIS_ID) const
{
	assert(RPCPtr);
	if (!Balance)
		throw InsufficientBalance("You have an empty balance!");

	if (DIS_ID == 0)
		throw GeneralAccountError("You need to specify an account to send to.");

	// Open (or create) other Discord User account to get the address
	std::string Wallet_Name = Poco::format(DISCORD_WALLET_MASK, DIS_ID);
	assert(RPCPtr->openWallet(Wallet_Name));
	const std::string DiscordUserAddress = RPCPtr->getAddress();

	if (DiscordUserAddress == MyAddress)
		throw GeneralAccountError("Don't transfer money to yourself.");

	// Now they we got the address reopen my account so we can send the money.
	Wallet_Name = Poco::format(DISCORD_WALLET_MASK, Discord_ID);
	assert(RPCPtr->openWallet(Wallet_Name));

	// Send the money
	return RPCPtr->sweepAll(Discord_ID, DiscordUserAddress);
}

TransferRet Account::transferMoneyToAddress(std::uint64_t amount, const std::string & address) const
{
	assert(RPCPtr);
	if (amount > Balance)
		throw InsufficientBalance(Poco::format("You are trying to send %f while only having %f!", amount / ITNS_OFFSET, Balance / ITNS_OFFSET));

	if (amount == 0)
		throw ZeroTransferAmount("You are trying to transfer a zero amount");

	if (address.empty())
		throw GeneralAccountError("You need to specify an address to send to.");

	if (address == MyAddress)
		throw GeneralAccountError("Don't transfer money to yourself.");

	// Send the money
	return RPCPtr->tranfer(Discord_ID, amount, address);
}

TransferRet Account::transferAllMoneyToAddress(const std::string& address) const
{
	assert(RPCPtr);
	if (Balance == 0)
		throw InsufficientBalance(Poco::format("You are trying to send all your money to an address while only having %f!", Balance / ITNS_OFFSET));

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
