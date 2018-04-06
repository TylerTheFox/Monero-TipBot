#include "Account.h"
#include "RPCException.h"
#include <iostream>

int main()
{
	try
	{
		Account Discord_User(9781/* Brandan */);
		std::cout << "My Address: " << Discord_User.getMyAddress() << "\nCurrent Balance: " << Discord_User.getBalance() << "\nCurrent Unlocked Balance: " << Discord_User.getUnlockedBalance() << '\n';
		auto a = Discord_User.transferMoneytoAnotherDiscordUser(0, 9781);
		a = Discord_User.transferMoneyToAddress(2000000000, "");
	}
	catch (AppGeneralException & exp)
	{
		std::cerr << exp.what() << " -- " << exp.getGeneralError();
	}
}