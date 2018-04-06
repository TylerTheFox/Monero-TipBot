#include "Account.h"
#include <string>
#include "AccountException.h"
#include "Poco/Format.h"
#include <cassert>

Account::Account(unsigned long long DIS_ID) : Discord_ID(DIS_ID)
{
	const std::string Wallet_Name = Poco::format(DISCORD_WALLET_MASK, DIS_ID);
	assert(RPCServ.openWallet(Wallet_Name));
	resyncAccount();
}

Account::~Account()
{
	// Close current wallet.
	assert(RPCServ.openWallet(VOID_WALLET));
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

TransferRet Account::transferMoneytoAnotherDiscordUser(unsigned long long amount, unsigned int DIS_ID) const
{
	if (amount > Balance)
	{
		throw InsufficientBalance(Poco::format("You are trying to send %Lu while only having %Lu!", amount, Balance));
	}

	if (amount == 0)
	{
		throw ZeroTransferAmount("You are trying to transfer a zero amount");
	}

	if (DIS_ID == 0)
	{
		throw GeneralAccountError("You need to specify an account to send to.");
	}

	// Open (or create) other Discord User account to get the address
	std::string Wallet_Name = Poco::format(DISCORD_WALLET_MASK, DIS_ID);
	assert(RPCServ.openWallet(Wallet_Name));
	const std::string DiscordUserAddress = RPCServ.getAddress();

	// Now they we got the address reopen my account so we can send the money.
	Wallet_Name = Poco::format(DISCORD_WALLET_MASK, Discord_ID);

	// Send the money
	return RPCServ.tranfer(Discord_ID, amount, DiscordUserAddress);
}

TransferRet Account::transferMoneyToAddress(unsigned long long amount, const std::string & address) const
{	
	if (amount > Balance)
	{
		throw InsufficientBalance(Poco::format("You are trying to send %Lu while only having %Lu!", amount, Balance));
	}

	if (amount == 0)
	{
		throw ZeroTransferAmount("You are trying to transfer a zero amount");
	}

	if (address.empty())
	{
		throw GeneralAccountError("You need to specify an address to send to.");
	}

	// Send the money
	return RPCServ.tranfer(Discord_ID, amount, address);
}

void Account::resyncAccount()
{
	const auto Bal = RPCServ.getBalance();
	Balance = Bal.Balance;
	UnlockedBalance = Bal.UnlockedBalance;
	MyAddress = RPCServ.getAddress();
}
