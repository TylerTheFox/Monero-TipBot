#include "RPC.h"

#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/SocketStream.h"
#include "Poco/StreamCopier.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include <Poco/Net/HTTPResponse.h>
#include <assert.h>

namespace RPC
{
	const Poco::DynamicStruct getDataFromRPC(unsigned int id, const std::string & method, const Poco::DynamicStruct & args)
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

	BalanceRet getBalance(int id)
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


	std::string getAddress(int id)
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

	unsigned int getBlockHeight(int id)
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

	TransferRet tranfer(int id, double amount, const std::string address)
	{
		TransferRet ret;

		// Building JSON string
		Poco::DynamicStruct params;
		Poco::Dynamic::Array destinations;

		Poco::DynamicStruct object;
		object["amount"] = amount * ITNS_OFFSET;
		object["address"] = address;
		destinations.push_back(object);

		params["destinations"] = destinations;
		params["mixin"] = Poco::format("%d", DEFAULT_MIXIN);
		params["get_tx_key"] = true;

		auto json = getDataFromRPC(id, RPC_METHOD_TRANSFER, params);
		auto result = json["result"].extract<Poco::DynamicStruct>();

		ret.fee			= result["fee"].convert<unsigned long long>();
		ret.tx_hash		= result["tx_hash"].toString();
		ret.tx_hash		= result["tx_key"].toString();

		return ret;
	}
}