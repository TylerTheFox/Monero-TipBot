#include "RPC.h"
#include <iomanip>

int main()
{
	auto bal = RPC::getBalance(0);

	std::cout << "Current Balance: " << std::setprecision(15) << (double)(bal.Balance / ITNS_OFFSET) << "\nUnlocked Balance: " << std::setprecision(15) << (double)(bal.UnlockedBalance / ITNS_OFFSET) << '\n';
	std::cout << "Current Block Height: " << RPC::getBlockHeight(0) << '\n';
	std::cout << "My Address: " << RPC::getAddress(0) << '\n';

	std::cout << "Testing Transfering of 10 ITNS\n\n";
	auto trx = RPC::tranfer(0, 10, "iz5ZrkSjiYiCMMzPKY8JANbHuyChEHh8aEVHNCcRa2nFaSKPqKwGCGuUMUMNWRyTNKewpk9vHFTVsHu32X3P8QJD21mfWJogf");

	std::cout
		<< "Transfer Complete!\n"
		<< "Fee: " << std::setprecision(15) << (double)(trx.fee / ITNS_OFFSET) << '\n'
		<< "TX Hash: " << trx.tx_hash << '\n'
		<< "TX Key: " << trx.tx_key << '\n';

	while (1);
}