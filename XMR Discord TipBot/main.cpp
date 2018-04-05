#include "RPC.h"
#include <iomanip>

int main()
{
	RPC CurrRPC("discord");

	auto bal = CurrRPC.getBalance(0);

	std::cout << "Current Balance: " << std::setprecision(15) << (double)(bal.Balance / ITNS_OFFSET) << "\nUnlocked Balance: " << std::setprecision(15) << (double)(bal.UnlockedBalance / ITNS_OFFSET) << '\n';
	std::cout << "Current Block Height: " << CurrRPC.getBlockHeight(0) << '\n';
	std::cout << "My Address: " << CurrRPC.getAddress(0) << '\n';

	auto ts = CurrRPC.getTransfers(0);

	std::cout << "Printing Incoming Transactions... \n";

	for (auto tx : ts)
	{
		std::cout << "Discord ID: " << tx.payment_id << " Amount: " << tx.amount << " Block Height: " << tx.block_height << '\n';
	}

	std::cout << "Testing Transfering of 20 ITNS\n\n";
	auto trx = CurrRPC.tranfer(0, 9781 /* Brandan's Discord ID */, 20 * ITNS_OFFSET, "iz5ZrkSjiYiCMMzPKY8JANbHuyChEHh8aEVHNCcRa2nFaSKPqKwGCGuUMUMNWRyTNKewpk9vHFTVsHu32X3P8QJD21mfWJogf");

	std::cout
		<< "Transfer Complete!\n"
		<< "Fee: " << std::setprecision(15) << (double)(trx.fee / ITNS_OFFSET) << '\n'
		<< "TX Hash: " << trx.tx_hash << '\n'
		<< "TX Key: " << trx.tx_key << '\n';
}