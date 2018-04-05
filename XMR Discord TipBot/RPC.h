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
#define RPC_HOSTNAME			"127.0.0.1"
#define RPC_PORT				8333

	// Vars
#define RPC_JSON				"/json_rpc"
#define ITNS_OFFSET				100000000.0

	// Methods
#define RPC_METHOD_GET_BALANCE "getbalance"

	struct BalanceRet
	{
		unsigned long long Balance;
		unsigned long long UnlockedBalance;
	};

	// Helper Function
	extern const Poco::DynamicStruct getDataFromRPC(unsigned int id, const std::string & method, const Poco::DynamicStruct & args);

	extern struct BalanceRet getBalance(int id);

}