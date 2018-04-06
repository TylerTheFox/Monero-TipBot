#include "Account.h"
#include <string>

#include "Poco/Format.h"

Account::Account(unsigned int Discord_ID)
{
	std::string Wallet_Name = Poco::format("Discord-User-%u", Discord_ID);
	RPCServ.openWallet(Wallet_Name);
	resyncAccount();
}

Account::~Account()
{
	// Close current wallet.
	RPCServ.openWallet("VOID-WALLET");
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

void Account::resyncAccount()
{
	auto Bal = RPCServ.getBalance();
	Balance = Bal.Balance;
	UnlockedBalance = Bal.UnlockedBalance;
	MyAddress = RPCServ.getAddress();
}
