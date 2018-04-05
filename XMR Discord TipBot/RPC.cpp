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
		Poco::DynamicStruct data;
		Poco::Dynamic::Array objects;

		objects.push_back(args);

		data["jsonrpc"] = "2.0";
		data["id"] = Poco::format("%u", id);
		data["method"] = method;
		data["params"] = objects;

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
		BalanceRet ret = {};
		auto json = getDataFromRPC(0, RPC_METHOD_GET_BALANCE, {});

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

}