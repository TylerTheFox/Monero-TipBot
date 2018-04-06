#pragma once
#include "RPC.h"
#include <string>

class Account
{
public:

	Account() = delete;
	Account(unsigned int Discord_ID);
	~Account();

	unsigned long long		getBalance() const;
	unsigned long long		getUnlockedBalance() const;
	const std::string &		getMyAddress() const;
private:
	RPC						RPCServ;
	std::string				MyAddress;
	unsigned long long		Balance;
	unsigned long long		UnlockedBalance;

	void					resyncAccount();
};