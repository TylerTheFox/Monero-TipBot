#pragma once
#include "RPC.h"
#include <string>

#define DISCORD_WALLET_MASK "Discord-User-%u"
#define VOID_WALLET			"VOID_WALLET" // This is opened when the account is closed.

class Account
{
public:
	Account() = delete;
	Account(unsigned int Discord_ID);
	~Account();

	unsigned long long		getBalance() const;
	unsigned long long		getUnlockedBalance() const;
	const std::string &		getMyAddress() const;

	TransferRet				transferMoneytoAnotherDiscordUser(unsigned long long amount, unsigned int Discord_ID) const;
	TransferRet				transferMoneyToAddress(unsigned long long amount, const std::string & address) const;
private:
	RPC						RPCServ;
	unsigned int			Discord_ID;
	std::string				MyAddress;
	unsigned long long		Balance{};
	unsigned long long		UnlockedBalance{};

	void					resyncAccount();
};