/*
Copyright(C) 2018 Brandan Tyler Lasley

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.
*/
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
#include <cassert>

void RPC::handleNetworkError(const std::string & msg)
{
	throw RPCConnectionError(msg);
}

void RPC::handleRPCError(Poco::DynamicStruct error)
{
	throw RPCGeneralError(error["code"].toString(), error["message"].toString());
}

Poco::DynamicStruct RPC::getDataFromRPC(const std::string & method, const Poco::DynamicStruct & args, int id)
{
	// Building JSON string
	Poco::DynamicStruct data;

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

BalanceRet RPC::getBalance(int id)
/* Global Wallet Balance */
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

	ret.Balance = result["balance"].convert<std::uint64_t>();
	ret.UnlockedBalance = result["unlocked_balance"].convert<std::uint64_t>();

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

TransferRet RPC::tranfer(std::uint64_t payment_id, std::uint64_t amount, const std::string & address, int id)
{
	TransferRet ret;

	// Building JSON string
	Poco::DynamicStruct params;
	Poco::Dynamic::Array destinations;

	Poco::DynamicStruct object;
	object["amount"] = amount;
	object["address"] = address;
	destinations.push_back(object);

	params["payment_id"] = Poco::format("%064Lu", payment_id);
	params["destinations"] = destinations;
	params["mixin"] = DEFAULT_MIXIN;
	params["get_tx_key"] = true;
	params["unlock_time"] = 0;

	auto json = getDataFromRPC(RPC_METHOD_TRANSFER, params, id);

	// Ensure RPC is happy.
	if (!json["error"].isEmpty())
	{
		handleRPCError(json["error"].extract<Poco::DynamicStruct>());
		return {};
	}

	auto result = json["result"].extract<Poco::DynamicStruct>();

	ret.fee = result["fee"].convert<std::uint64_t>();
	ret.tx_hash = result["tx_hash"].toString();
	ret.tx_key = result["tx_key"].toString();

	return ret;
}

TransferRet RPC::sweepAll(std::uint64_t payment_id, const std::string & address, int id)
{
	TransferRet ret;

	// Building JSON string
	Poco::DynamicStruct params;

	params["address"] = address;
	params["payment_id"] = Poco::format("%064Lu", payment_id);
	params["mixin"] = DEFAULT_MIXIN;
	params["get_tx_keys"] = true;
	params["unlock_time"] = 0;

	auto json = getDataFromRPC(RPC_METHOD_SWEEP_ALL, params, id);

	// Ensure RPC is happy.
	if (!json["error"].isEmpty())
	{
		handleRPCError(json["error"].extract<Poco::DynamicStruct>());
		return {};
	}

	auto result = json["result"].extract<Poco::DynamicStruct>();

	auto tx_hash_list = result["tx_hash_list"].extract<Poco::Dynamic::Array>();
	auto tx_key_list = result["tx_key_list"].extract<Poco::Dynamic::Array>();

	ret.fee			= 0;
	ret.tx_hash		= tx_hash_list[0].toString();
	ret.tx_key		= tx_hash_list[0].toString();

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

	// Loop though the IN and OUT transaction list and add it to the 
	// tx_in and tx_out vector.
	const auto processList = [&](const std::string & inout)
	{
		if (!json["result"][inout].isEmpty())
		{
			auto result = json["result"][inout].extract<Poco::Dynamic::Array>();

			struct TransferItem ts = {};
			for (auto it : result)
			{
				ts.tx_hash = it["txid"].toString();
				ts.payment_id = it["payment_id"].convert<std::uint64_t>();
				ts.block_height = it["height"].convert<unsigned int>();
				if (it["amount"].isInteger())
				{
					ts.amount = it["amount"].convert<std::uint64_t>();
				}
				else ts.amount = 0;
				if (inout == "in")
					incomingTransactions.tx_in.insert(ts);
				else
					incomingTransactions.tx_out.insert(ts);
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
		assert(createWallet(name, password));

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

void RPC::stopWallet(int id)
// This doesn't close the wallet but the RPC.
{
	auto json = getDataFromRPC(RPC_METHOD_CLOSE_RPC, {}, id);

	if (!json["error"].isEmpty())
	{
		handleRPCError(json["error"].extract<Poco::DynamicStruct>());
	}
}

void RPC::store(int id)
{
	auto json = getDataFromRPC(RPC_METHOD_STORE, {}, id);

	/*
	 * Errors in store aren't that important and causes the tipbot
	 * To get stuck in an infinite error cycle.
	 */

	/*if (!json["error"].isEmpty())
	{
		handleRPCError(json["error"].extract<Poco::DynamicStruct>());
	}*/
}
