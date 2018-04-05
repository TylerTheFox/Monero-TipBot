#pragma once
#include <string>
#include <vector>
#include <string>
#include <sstream>

#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Object.h>
#include <Poco/Dynamic/Var.h>
#include "Poco/Process.h"
#include "Poco/Pipe.h"

// Server Settings
#define RPC_HOSTNAME							"127.0.0.1"
#define RPC_PORT								8333

// Vars
#define RPC_JSON								"/json_rpc"
#define ITNS_OFFSET								100000000.0 // 1 x 10^8
#define DEFAULT_MIXIN							4

// Methods
#define RPC_METHOD_GET_BALANCE					"getbalance"
#define RPC_METHOD_GET_ADDRESS					"getaddress"
#define RPC_METHOD_GET_BLK_HEIGHT				"getheight"
#define RPC_METHOD_TRANSFER						"transfer"
#define RPC_METHOD_GET_TRANSFERS				"get_transfers"

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

struct TransferList
{
	unsigned int payment_id;
	unsigned long long amount;
	unsigned int block_height;
};

extern bool RPC_RUNNING;

class RPC
{
public:
	RPC(const std::string & wallet);
	~RPC();

	struct BalanceRet					getBalance(int id);
	std::string							getAddress(int id);
	unsigned int						getBlockHeight(int id);
	TransferRet							tranfer(int id, int payment_id, double amount, const std::string address);
	std::vector<struct TransferList>	getTransfers(int id);
private:
	static bool RPC_RUNNING;
	Poco::Pipe	rpc_pipe;
	int			rpc_pid;

	const Poco::DynamicStruct			getDataFromRPC(unsigned int id, const std::string & method, const Poco::DynamicStruct & args);
};