#pragma once
#include <string>
#include <vector>
#include <string>

#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Object.h>

// Server Settings
#define RPC_FILENAME							"intense-wallet-rpc.exe"
#define RPC_HOSTNAME							"127.0.0.1"
#define RPC_PORT								8333
#define DAEMON_ADDRESS							"127.0.0.1:48782"

// Vars
#define RPC_JSON								"/json_rpc"
#define WALLET_PATH								"./Wallets/"
#define ITNS_OFFSET								100000000.0 // 1 x 10^8
#define DEFAULT_MIXIN							4

// Methods
#define RPC_METHOD_GET_BALANCE					"getbalance"
#define RPC_METHOD_GET_ADDRESS					"getaddress"
#define RPC_METHOD_GET_BLK_HEIGHT				"getheight"
#define RPC_METHOD_TRANSFER						"transfer"
#define RPC_METHOD_SWEEP_ALL					"sweep_all"
#define RPC_METHOD_GET_TRANSFERS				"get_transfers"
#define RPC_METHOD_CREATE_WALLET				"create_wallet"
#define RPC_METHOD_OPEN_WALLET					"open_wallet"
#define RPC_METHOD_CLOSE_RPC					"stop_wallet"
#define RPC_METHOD_STORE						"store"

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

struct TransferItem
{
	unsigned long long amount;
	unsigned int payment_id;
	unsigned int block_height;
};

struct TransferList
{
	std::vector<struct TransferItem> tx_in;
	std::vector<struct TransferItem> tx_out;
};

class RPC
{
public:
	RPC() = default;

	struct BalanceRet					getBalance(int id = 0) const;
	std::string							getAddress(int id = 0) const;
	unsigned int						getBlockHeight(int id = 0) const;
	TransferRet							tranfer(unsigned long long payment_id, unsigned long long amount, const std::string & address, int id = 0) const;
	TransferRet							sweepAll(unsigned long long payment_id, const std::string & address, int id = 0) const;
	TransferList						getTransfers(int id = 0);
	bool								createWallet(const std::string & name, const std::string & password = {}, const std::string & language = "English", int id = 0) const;
	bool								openWallet(const std::string & name, const std::string & password = {}, int id = 0) const;
	void								stopWallet(int id = 0) const;
	void								store(int id = 0) const;

private:
	static void							handleNetworkError(const std::string & msg);
	static void							handleRPCError(Poco::DynamicStruct error);
	Poco::DynamicStruct					getDataFromRPC(const std::string & method, const Poco::DynamicStruct & args, int id = 0) const;
};