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
	~Account();

	unsigned long long		getBalance() const;
	unsigned long long		getUnlockedBalance() const;
	const std::string &		getMyAddress() const;

	TransferRet				transferMoneytoAnotherDiscordUser(unsigned long long amount, unsigned long long Discord_ID) const;
	TransferRet				transferMoneyToAddress(unsigned long long amount, const std::string & address) const;
	TransferRet				transferAllMoneyToAddress(const std::string & address) const;
private:
	RPC						RPCServ;
	unsigned long long		Discord_ID;
	unsigned long long		Balance{};
	unsigned long long		UnlockedBalance{};
	std::string				MyAddress;

	void					resyncAccount();
};