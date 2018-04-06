#include "Account.h"
#include <string>
#include "AccountException.h"
#include "Poco/Format.h"

Account::Account(unsigned int DIS_ID)
{
	Discord_ID = DIS_ID;
	std::string Wallet_Name = Poco::format(DISCORD_WALLET_MASK, DIS_ID);
	RPCServ.openWallet(Wallet_Name);
	resyncAccount();
}

Account::~Account()
{
	// Close current wallet.
	RPCServ.openWallet(VOID_WALLET);
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

void Account::transferMoneytoAnotherDiscordUser(unsigned long long amount, unsigned int DIS_ID)
{
	if (amount > Balance)
	{
		InsufficientBalance inBal(Poco::format("You are trying to send %Lu while only having %Lu!", amount, Balance));
		throw inBal;
	}

	if (amount == 0)
	{
		ZeroTransferAmount TrxErr("You are trying to transfer a zero amount");
		throw TrxErr;
	}

	if (DIS_ID == 0)
	{
		GeneralAccountError GenErr("You need to specify an account to send to.");
		throw GenErr;
	}

	// Open (or create) other Discord User account to get the address
	std::string Wallet_Name = Poco::format(DISCORD_WALLET_MASK, DIS_ID);
	RPCServ.openWallet(Wallet_Name);
	std::string DiscordUserAddress = RPCServ.getAddress();

	// Now they we got the address reopen my account so we can send the money.
	Wallet_Name = Poco::format(DISCORD_WALLET_MASK, Discord_ID);

	// Send the money
	RPCServ.tranfer(Discord_ID, amount, DiscordUserAddress);
}

void Account::transferMoneyToAddress(unsigned long long amount, const std::string & address)
{	
	if (amount > Balance)
	{
		InsufficientBalance inBal(Poco::format("You are trying to send %Lu while only having %Lu!", amount, Balance));
		throw inBal;
	}

	if (amount == 0)
	{
		ZeroTransferAmount TrxErr("You are trying to transfer a zero amount");
		throw TrxErr;
	}

	if (address.empty())
	{
		GeneralAccountError GenErr("You need to specify an address to send to.");
		throw GenErr;
	}

	// Send the money
	RPCServ.tranfer(Discord_ID, amount, address);
}

void Account::resyncAccount()
{
	auto Bal = RPCServ.getBalance();
	Balance = Bal.Balance;
	UnlockedBalance = Bal.UnlockedBalance;
	MyAddress = RPCServ.getAddress();
}
