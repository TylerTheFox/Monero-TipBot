#include "Account.h"
#include "RPCException.h"
#include <iostream>
#include <iomanip>

int main()
{
	try
	{
		Account Discord_User(9781/* Brandan */);
		std::cout << "My Address: " << Discord_User.getMyAddress() << "\nCurrent Balance: " << Discord_User.getBalance() << "\nCurrent Unlocked Balance: " << Discord_User.getUnlockedBalance() << '\n';
	}
	catch (RPCExceptiion & exp)
	{
		std::cerr << exp.what() << " -- " << exp.getGeneralError();
	}
}