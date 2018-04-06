#include "RPC.h"
#include "Util.h"
#include "RPCException.h"

#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/SocketStream.h"
#include "Poco/StreamCopier.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include <Poco/Net/HTTPResponse.h>
#include "Poco/Random.h"
#include "Poco/RandomStream.h"
#include "Poco/File.h"
#include <assert.h>

#include <fstream>

#if RPC_AUTO_START
bool RPC::RPC_RUNNING = false;
#endif

RPC::RPC(const std::string & wallet)
{
#if RPC_AUTO_START
	if (!RPC_RUNNING)
	{
		RPC_RUNNING = true;

		std::vector<std::string> args;
		args.push_back("--wallet-dir");
		args.push_back(WALLET_PATH);
		args.push_back("--rpc-bind-port");
		args.push_back("8333");
		args.push_back("--daemon-address");
		args.push_back("127.0.0.1:48782");
		args.push_back("--disable-rpc-login");
		args.push_back("--trusted-daemon");

		Poco::ProcessHandle rpc_handle = Poco::Process::launch("intense-wallet-rpc.exe", args, 0, &rpc_pipe, 0);
		rpc_pid = rpc_handle.id();
		assert(rpc_pid > 0);
	}
#endif
}

RPC::~RPC()
{
#if RPC_AUTO_START
	if (!RPC_RUNNING)
	{
		assert(rpc_pid);
		Poco::Process::requestTermination(rpc_pid);
		rpc_pid = 0;
		RPC_RUNNING = false;
	}
#endif
}

void RPC::handleNetworkError(const std::string & msg)
{
	RPCConnectionFailed exp(msg);
	throw exp;
}

void RPC::handleRPCError(Poco::DynamicStruct error)
{
	RPCGeneralError exp(error["code"].toString(), error["message"].toString());
	throw exp;
}

const Poco::DynamicStruct RPC::getDataFromRPC(const std::string & method, const Poco::DynamicStruct & args, int id)
{
	// Building JSON string
	Poco::DynamicStruct data;
	Poco::Dynamic::Array objects;

	data["jsonrpc"] = "2.0";
	data["id"] = Poco::format("%d", id);
	data["method"] = method;
	data["params"] = args;

	try
	{
		// Networking
		std::string body = data.toString();
		Poco::Net::HTTPClientSession s(RPC_HOSTNAME, RPC_PORT);
		Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, RPC_JSON);
		Poco::Net::HTTPResponse res;
		Poco::JSON::Parser parser;
		std::stringstream ss;

		// Setup Request
		request.setContentType("application/json");
		request.setContentLength(body.length());
		s.sendRequest(request) << body; // Send POST body

		// Recieve Return Data
		std::istream &is = s.receiveResponse(res);
		Poco::StreamCopier::copyStream(is, ss);

		// Parse JSON and return
		auto JSONResult = parser.parse(ss.str());
		Poco::JSON::Object::Ptr object = JSONResult.extract<Poco::JSON::Object::Ptr>();
		return Poco::DynamicStruct(*object);
	}
	catch (const Poco::Exception & exp)
	{
		handleNetworkError(exp.what());
	}
	return {};
}

BalanceRet RPC::getBalance(int id) /* Global Wallet Balance */
{
	BalanceRet ret;
	auto json = getDataFromRPC(RPC_METHOD_GET_BALANCE, {}, id);

	// Ensure RPC is happy.
	if (!json["error"].isEmpty())
		handleRPCError(json["error"].extract<Poco::DynamicStruct>());

	// Check if we returned a result
	assert(json["result"].isStruct());

	// Check if id is valid
	assert(json["id"].convert<int>() == id);

	// Get Balance / Unlocked Balance.
	auto result = json["result"].extract<Poco::DynamicStruct>();

	ret.Balance = result["balance"].convert<unsigned long long>();
	ret.UnlockedBalance = result["unlocked_balance"].convert<unsigned long long>();

	return ret;
}

std::string RPC::getAddress(int id)
{
	auto json = getDataFromRPC(RPC_METHOD_GET_ADDRESS, {}, id);

	// Ensure RPC is happy.
	if (!json["error"].isEmpty())
		handleRPCError(json["error"].extract<Poco::DynamicStruct>());

	// Check if we returned a result
	assert(json["result"].isStruct());

	// Check if id is valid
	assert(json["id"].convert<int>() == id);

	// Get Address
	auto result = json["result"].extract<Poco::DynamicStruct>();
	return result["address"].toString();
}

unsigned int RPC::getBlockHeight(int id)
{
	auto json = getDataFromRPC(RPC_METHOD_GET_BLK_HEIGHT, {}, id);

	// Ensure RPC is happy.
	if (!json["error"].isEmpty())
		handleRPCError(json["error"].extract<Poco::DynamicStruct>());

	// Check if we returned a result
	assert(json["result"].isStruct());

	// Check if id is valid
	assert(json["id"].convert<int>() == id);

	// Get Block Height
	auto result = json["result"].extract<Poco::DynamicStruct>();
	return result["height"].convert<unsigned int>();
}

TransferRet RPC::tranfer(int payment_id, double amount, const std::string address, int id)
{
	TransferRet ret;

	// Building JSON string
	Poco::DynamicStruct params;
	Poco::Dynamic::Array destinations;

	Poco::DynamicStruct object;
	object["amount"] = amount;
	object["address"] = address;
	destinations.push_back(object);

	params["payment_id"] = Poco::format("%016d", payment_id/* Discord ID */);;
	params["destinations"] = destinations;
	params["mixin"] = DEFAULT_MIXIN;
	params["get_tx_key"] = true;

	auto json = getDataFromRPC(RPC_METHOD_TRANSFER, params, id);

	// Ensure RPC is happy.
	if (!json["error"].isEmpty())
	{
		handleRPCError(json["error"].extract<Poco::DynamicStruct>());
		return {};
	}

	auto result = json["result"].extract<Poco::DynamicStruct>();

	ret.fee = result["fee"].convert<unsigned long long>();
	ret.tx_hash = result["tx_hash"].toString();
	ret.tx_hash = result["tx_key"].toString();

	return ret;
}

TransferList RPC::getTransfers(int id)
{
	struct TransferList incomingTransactions;

	Poco::DynamicStruct params;
	params["in"] = true;
	params["out"] = true;
	auto json = getDataFromRPC(RPC_METHOD_GET_TRANSFERS, params, id);

	// Ensure RPC is happy.
	if (!json["error"].isEmpty())
		handleRPCError(json["error"].extract<Poco::DynamicStruct>());

	// Check if we returned a result
	assert(json["result"].isStruct());

	// Check if id is valid
	assert(json["id"].convert<int>() == id);

	auto processList = [&](const std::string & inout)
	{
		if (!json["result"][inout].isEmpty())
		{
			auto result = json["result"][inout].extract<Poco::Dynamic::Array>();

			struct TransferItem ts;
			for (auto it : result)
			{
				ts.payment_id = it["payment_id"].convert<unsigned int>();
				ts.block_height = it["height"].convert<unsigned int>();
				if (it["amount"].isInteger())
				{
					ts.amount = it["amount"].convert<unsigned long long>();
				}
				else ts.amount = 0;
				if (inout == "in")
					incomingTransactions.tx_in.push_back(ts);
				else
					incomingTransactions.tx_out.push_back(ts);
			}
		}
	};

	processList("in");
	processList("out");

	return incomingTransactions;
}

bool RPC::createWallet(const std::string & name, const std::string & password, const std::string & language, int id)
{
	// Ensure we dont overwrite a wallet.
	assert(!Util::doesWalletExist(WALLET_PATH + name));

	Poco::DynamicStruct data;

	data["filename"] = name;
	data["password"] = password;
	data["language"] = language;

	auto json = getDataFromRPC(RPC_METHOD_CREATE_WALLET, data, id);

	// Ensure RPC is happy.
	if (!json["error"].isEmpty())
	{
		handleRPCError(json["error"].extract<Poco::DynamicStruct>());
		return false;
	}


	// Ensure Wallet Exists
	return Util::doesWalletExist(WALLET_PATH + name);
}

bool RPC::openWallet(const std::string & name, const std::string & password, int id)
{
	// Ensure Wallet Exists
	if (!Util::doesWalletExist(WALLET_PATH + name))
		createWallet(name, password);

	Poco::DynamicStruct data;

	data["filename"] = name;
	data["password"] = password;

	auto json = getDataFromRPC(RPC_METHOD_OPEN_WALLET, data, id);

	if (!json["error"].isEmpty())
	{
		handleRPCError(json["error"].extract<Poco::DynamicStruct>());
		return false;
	}

	return true;
}

void RPC::closeWallet(int id)
{
	auto json = getDataFromRPC(RPC_METHOD_CLOSE_WALLET, {}, id);

	if (!json["error"].isEmpty())
	{
		handleRPCError(json["error"].extract<Poco::DynamicStruct>());
	}
}
