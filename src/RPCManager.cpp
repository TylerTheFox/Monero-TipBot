#include "RPCManager.h"
#include "Poco/Process.h"
#include "Poco/Thread.h"
#include "Poco/Timespan.h"
#include "Util.h"
#include <cassert>
#include "Discord.h"
#include "RPCException.h"
#include "Poco/File.h"
#include <fstream>

RPCManager			 RPCMan;
RPCProc* RPCManager::BotRPCProc;

RPCManager::RPCManager() : currPortNum(STARTING_PORT_NUMBER), DiscordPtr(nullptr)
{
	BotRPCProc = new RPCProc(SpinUpNewRPC(0));
}

RPCManager::~RPCManager()
{
	// Save blockchain on exit.
	SaveWallets();
	save();
}

void RPCManager::setDiscordPtr(ITNS_TIPBOT* ptr)
{
	DiscordPtr = ptr;
}

bool RPCManager::isRPCRunning(DiscordID id)
{
	return (RPCMap.count(id) == 1);
}

time_t RPCManager::getTimeStarted(DiscordID id)
{
	return RPCMap[id].timestamp.epochTime();
}

Account& RPCManager::getAccount(DiscordID id)
{
	mu.lock();

	if (RPCMap.count(id) == 0)
	{
		if (RPCMap.size() <= MAX_RPC_LIMIT)
			RPCMap[id] = SpinUpNewRPC(id);
		else
			RPCMap[id] = FindOldestRPC();

		// Setup Account
		RPCMap[id].MyAccount.open(id, &RPCMap[id].MyRPC);

		// Wait for RPC to respond
		waitForRPCToRespond(id);

		// Open Wallet
		assert(RPCMap[id].MyRPC.openWallet(Util::getWalletStrFromIID(id)));

		// Get transactions
		RPCMap[id].Transactions = RPCMap[id].MyRPC.getTransfers();
	}

	// Account Resync
	RPCMap[id].MyAccount.resyncAccount();

	mu.unlock();

	return RPCMap[id].MyAccount;
}

const TransferList RPCManager::getTransfers(DiscordID id)
{
	if (RPCMap.count(id))
		return RPCMap[id].Transactions;
	return {};
}

void RPCManager::run()
{
	const Poco::Timestamp timeStarted;
	while (true)
	{
		Poco::Timespan timer(Poco::Timestamp() - timeStarted);
		if (DiscordPtr)
		{
			try
			{
				if ((timer.seconds() % SEARCH_FOR_NEW_TRANSACTIONS_TIME) == 0)
				{

					processNewTransactions();
				}

				if ((timer.minutes() > 0) && (timer.seconds() == 0) && (timer.minutes() % BLOCKCHAIN_SAVE_TIME) == 0)
				{
					SaveWallets();
				}

				if ((timer.seconds() % RPC_WALLETS_SAVE_TIME) == 0)
				{
					save();
				}
			}
			catch (const Poco::Exception & exp)
			{
				std::cout << "Poco Error: " << exp.what();
			}
			catch (AppGeneralException & exp)
			{
				std::cout << "App Error: " << exp.what();
			}
		}
		Poco::Thread::sleep(1000);
	}
}

void RPCManager::processNewTransactions()
{
	mu.lock();
	std::cout << "Searching for new transactions...\n";
	Poco::JSON::Parser parser;
	Poco::JSON::Object::Ptr object;
	std::string clientID;
	std::vector<struct TransferItem> diff;
	TransferList newTransactions;

	for (auto & account : this->RPCMap)
	{
		newTransactions = account.second.MyRPC.getTransfers();

		std::set_difference(newTransactions.tx_in.begin(), newTransactions.tx_in.end(), account.second.Transactions.tx_in.begin(), account.second.Transactions.tx_in.end(),
			std::inserter(diff, diff.begin()));

		if (!diff.empty())
		{
			try
			{
				auto response = DiscordPtr->createDirectMessageChannel(Poco::format("%Lu", account.first));
				object = parser.parse(response.text).extract<Poco::JSON::Object::Ptr>();
				clientID = object->getValue<std::string>("id");

				for (auto newTx : diff)
				{
					DiscordPtr->sendMessage(clientID, Poco::format("You've recieved money! %f ITNS :money_with_wings:", newTx.amount / ITNS_OFFSET));
					std::cout << Poco::format("User %Lu recived %f ITNS\n", account.first, newTx.amount / ITNS_OFFSET);
				}

				account.second.Transactions = newTransactions;

				diff.clear();
			}
			catch (const Poco::Exception & exp)
			{
				diff.clear();
			}
		}
	}
	mu.unlock();
}

const RPC & RPCManager::getGlobalBotRPC()
{
	return BotRPCProc->MyRPC;
}

void RPCManager::save()
{
	mu.lock();
	std::ofstream out(RPC_DATABASE_FILENAME, std::ios::trunc);
	if (out.is_open())
	{
		std::cout << "Saving wallet data to disk...\n";
		{
			cereal::JSONOutputArchive ar(out);
			ar(CEREAL_NVP(currPortNum), CEREAL_NVP(RPCMap));
		}
		out.close();
	}
	mu.unlock();
}

void RPCManager::load()
{
	mu.lock();
	Poco::File RPCFile(RPC_DATABASE_FILENAME);
	if (RPCFile.exists())
	{
		std::cout << "Loading wallet files and spinning up RPCs..\n";
		std::ifstream in(RPC_DATABASE_FILENAME);
		if (in.is_open())
		{
			{
				cereal::JSONInputArchive ar(in);
				ar(CEREAL_NVP(currPortNum), CEREAL_NVP(RPCMap));
			}
			in.close();
			ReloadSavedRPCs();
		}
	}
	mu.unlock();
}

RPCProc RPCManager::SpinUpNewRPC(DiscordID id)
{
	RPCProc RPC_DATA;
	RPC_DATA.pid = LaunchRPC(currPortNum);
	RPC_DATA.MyAccount.open(id, &RPC_DATA.MyRPC);
	RPC_DATA.MyRPC.open(currPortNum);

	currPortNum++;
	return RPC_DATA;
}

void RPCManager::SpinDownRPC(DiscordID id)
{
	RPCMap[id].MyRPC.stopWallet();
	RPCMap.erase(id);
}

RPCProc& RPCManager::FindOldestRPC()
{
	auto it = std::min_element(RPCMap.begin(), RPCMap.end(),
		[](decltype(RPCMap)::value_type& l, decltype(RPCMap)::value_type& r) -> bool { return l.second.timestamp < r.second.timestamp; });
		
	return it->second;
}

void RPCManager::SaveWallets()
{
	mu.lock();
	std::cout << "Saving blockchain...\n";

	// Save blockchain on exit.
	for (auto account : this->RPCMap)
		account.second.MyRPC.store();
	mu.unlock();
}

void RPCManager::ReloadSavedRPCs()
{
	for (auto & wallets : RPCMap)
	{
		// Launch RPC
		wallets.second.pid = LaunchRPC(wallets.second.MyRPC.getPort());

		// Setup Accounts
		wallets.second.MyAccount.open(wallets.first, &wallets.second.MyRPC);

		// Wait for RPC to respond
		waitForRPCToRespond(wallets.first);

		// Open Wallet
		assert(wallets.second.MyRPC.openWallet(Util::getWalletStrFromIID(wallets.first)));

		// Get transactions
		wallets.second.Transactions = wallets.second.MyRPC.getTransfers();
	}
}

unsigned int RPCManager::LaunchRPC(unsigned short port)
{
	std::vector<std::string> args;
	args.emplace_back("--wallet-dir");
	args.emplace_back(WALLET_PATH);
	args.emplace_back("--rpc-bind-port");
	args.push_back(Poco::format("%?i", port));
	args.emplace_back("--daemon-address");
	args.emplace_back(DAEMON_ADDRESS);
	args.emplace_back("--disable-rpc-login");
	args.emplace_back("--trusted-daemon");

	Poco::ProcessHandle rpc_handle = Poco::Process::launch(RPC_FILENAME, args, nullptr, nullptr, nullptr);
	return rpc_handle.id();
}

void RPCManager::waitForRPCToRespond(DiscordID id)
{
	// Wait for RPC to respond to requests.
	// This is because we need to ensure open_wallet actually gets called
	// and if RPC is still loading it'll just go into the void.
	bool waitForRPC = true;
	while (waitForRPC)
	{
		try
		{
			RPCMap[id].MyRPC.getBlockHeight(); // Query RPC until it responds.
			waitForRPC = false;
		}
		catch (RPCConnectionError & exp)
		{
			Poco::Thread::sleep(100);
		}
		catch (const Poco::Exception & exp) // Catch network exceptions.
		{
			Poco::Thread::sleep(100);
		}
		catch (RPCGeneralError & exp) // Some other error probably no wallet file
		{
			waitForRPC = false;
		}
	}
}