#include "RPCManager.h"
#include "Poco/Process.h"
#include "Poco/Thread.h"
#include "Poco/Timespan.h"
#include "Util.h"
#include <cassert>
#include "Discord.h"

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
		RPCMap[id].MyAccount.open(id, &RPCMap[id].MyRPC);

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

void RPCManager::run()
{
	const Poco::Timestamp timeStarted;
	while (true)
	{
		Poco::Timespan timer(Poco::Timestamp() - timeStarted);

		if (timer.seconds() > 0 && (timer.seconds() % 10) == 0)
		{
			processNewTransactions();
		}

		if (timer.minutes() > 0 && (timer.minutes() % 15) == 0)
		{
			SaveWallets();
		}
		Poco::Thread::sleep(1);
	}
}

void RPCManager::processNewTransactions()
{
	mu.lock();
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
				auto response = DiscordPtr->createDirectMessageChannel(Poco::format("%Lu", account.second.myID));
				object = parser.parse(response.text).extract<Poco::JSON::Object::Ptr>();
				clientID = object->getValue<std::string>("id");


				for (auto newTx : diff)
				{
					DiscordPtr->sendMessage(clientID, Poco::format("You've recieved money! %f ITNS :money_with_wings:", newTx.amount / ITNS_OFFSET));
					std::cout << Poco::format("User %Lu recived %f ITNS\n", account.second.myID,newTx.amount / ITNS_OFFSET);
				}

				account.second.Transactions = newTransactions;

				diff.clear();
			} 
			catch (const Poco::Exception & exp)
			{
				diff.clear();
			}
		}
		Poco::Thread::sleep(10);
	}
	mu.unlock();
}

const RPC & RPCManager::getGlobalBotRPC()
{
	return BotRPCProc->MyRPC;
}

RPCProc RPCManager::SpinUpNewRPC(DiscordID id)
{
	RPCProc RPC_DATA;
	std::vector<std::string> args;
	args.emplace_back("--wallet-dir");
	args.emplace_back(WALLET_PATH);
	args.emplace_back("--rpc-bind-port");
	args.push_back(Poco::format("%?i", currPortNum));
	args.emplace_back("--daemon-address");
	args.emplace_back(DAEMON_ADDRESS);
	args.emplace_back("--disable-rpc-login");
	args.emplace_back("--trusted-daemon");

	Poco::ProcessHandle rpc_handle = Poco::Process::launch(RPC_FILENAME, args, nullptr, nullptr, nullptr);

	RPC_DATA.myID = id;
	RPC_DATA.pid = rpc_handle.id();
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

	std::cout << it->second.myID;

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
