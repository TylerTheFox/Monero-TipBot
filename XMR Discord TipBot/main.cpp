#include "RPC.h"
#include <iomanip>

int main()
{
	auto bal = RPC::getBalance(0);

	std::cout << "Current Balance: " << std::setprecision(15) << (double)(bal.Balance / ITNS_OFFSET) << " Unlocked Balance: " << std::setprecision(15) << (double)(bal.UnlockedBalance / ITNS_OFFSET) << '\n';

	while (1);
}