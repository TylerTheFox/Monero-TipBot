#pragma once
#include <vector>
#include <string>
#include <sstream>

#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Object.h>
#include <Poco/Dynamic/Var.h>

namespace RPC
{
	// Server Settings
#define RPC_HOSTNAME							"127.0.0.1"
#define RPC_PORT								8333

	// Vars
#define RPC_JSON								"/json_rpc"
#define ITNS_OFFSET								100000000.0
#define DEFAULT_MIXIN							4

	// Methods
#define RPC_METHOD_GET_BALANCE					"getbalance"
#define RPC_METHOD_GET_ADDRESS					"getaddress"
#define RPC_METHOD_GET_BLK_HEIGHT				"getheight"
#define RPC_METHOD_TRANSFER						"transfer"

	struct BalanceRet
	{
		unsigned long long Balance;
		unsigned long long UnlockedBalance;
	};

	struct TransferRet
	{
		unsigned long long	fee;
		std::string			tx_hash;
		std::string			tx_key;
	};

	// Helper Function
	extern const Poco::DynamicStruct getDataFromRPC(unsigned int id, const std::string & method, const Poco::DynamicStruct & args);

	extern struct BalanceRet		getBalance(int id);
	extern std::string				getAddress(int id);
	extern unsigned int				getBlockHeight(int id);
	extern TransferRet				tranfer(int id, double amount, const std::string address);
}