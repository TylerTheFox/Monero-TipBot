#include "RPC.h"

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

bool RPC::RPC_RUNNING = false;

RPC::RPC(const std::string & wallet)
{
	// Auto start causing problems because RPC has to sync first
	// And we cannot detect if RPC is syncing or not (ie we crash).

	/*if (!RPC_RUNNING)
	{
		RPC_RUNNING = true;

		std::vector<std::string> args;
		args.push_back("--wallet-file");
		args.push_back(wallet);
		args.push_back("--password");
		args.push_back("EJmM5rtrj8n0KKwMqTE0GgLgWhHmrMQLK8PRxmMJnmRkkk0BaIs4udAxr9KbgcMLc20LnEhwx7zpzKjTmSYfx5BMtyMKQMQqHjj8UsJGz8wgqU1n1hGbF6EDSfGX2j5cPUvGpi0BaZ45kgHTdwU4a76s3HRtXhZLMb4dX33bb2WEKqMjXk0VF73OZ77nTewBhXDtRXfU");
		args.push_back("--rpc-bind-port");
		args.push_back("8333");
		args.push_back("--daemon-address");
		args.push_back("127.0.0.1:48782");
		args.push_back("--disable-rpc-login");
		args.push_back("--trusted-daemon");

		Poco::ProcessHandle rpc_handle = Poco::Process::launch("intense-wallet-rpc.exe", args, 0, &rpc_pipe, 0);
		rpc_pid = rpc_handle.id();
		assert(rpc_pid > 0);
	}*/
}

RPC::~RPC()
{
	assert(rpc_pid);
	Poco::Process::requestTermination(rpc_pid);
	rpc_pid = 0;
	RPC_RUNNING = false;
}

const Poco::DynamicStruct RPC::getDataFromRPC(unsigned int id, const std::string & method, const Poco::DynamicStruct & args)
{
	// Building JSON string
	Poco::DynamicStruct data;
	Poco::Dynamic::Array objects;

	data["jsonrpc"] = "2.0";
	data["id"] = Poco::format("%u", id);
	data["method"] = method;
	data["params"] = args;

	// Networking
	std::string body = data.toString();
	std::cout << body;
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

BalanceRet RPC::getBalance(int id) /* Global Wallet Balance */
{
	BalanceRet ret;
	auto json = getDataFromRPC(id, RPC_METHOD_GET_BALANCE, {});

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
	auto json = getDataFromRPC(id, RPC_METHOD_GET_ADDRESS, {});

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
	auto json = getDataFromRPC(id, RPC_METHOD_GET_BLK_HEIGHT, {});

	// Check if we returned a result
	assert(json["result"].isStruct());

	// Check if id is valid
	assert(json["id"].convert<int>() == id);

	// Get Balance / Unlocked Balance.
	auto result = json["result"].extract<Poco::DynamicStruct>();
	return result["height"].convert<unsigned int>();
}

TransferRet RPC::tranfer(int id, int payment_id, double amount, const std::string address)
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

	auto json = getDataFromRPC(id, RPC_METHOD_TRANSFER, params);
	auto result = json["result"].extract<Poco::DynamicStruct>();

	ret.fee = result["fee"].convert<unsigned long long>();
	ret.tx_hash = result["tx_hash"].toString();
	ret.tx_hash = result["tx_key"].toString();

	return ret;
}

std::vector<struct TransferList> RPC::getTransfers(int id)
{
	std::vector<struct TransferList> incomingTransactions;

	Poco::DynamicStruct params;
	params["in"] = true;
	//params["out"] = true;
	auto json = getDataFromRPC(id, RPC_METHOD_GET_TRANSFERS, params);

	// Check if we returned a result
	assert(json["result"].isStruct());

	// Check if id is valid
	assert(json["id"].convert<int>() == id);

	auto result = json["result"]["in"].extract<Poco::Dynamic::Array>();

	struct TransferList ts;
	for (auto it : result)
	{
		ts.payment_id = it["payment_id"].convert<unsigned int>();
		ts.block_height = it["height"].convert<unsigned int>();
		ts.amount = it["amount"].convert<unsigned long long>();
		incomingTransactions.push_back(ts);
	}

	return incomingTransactions;
}