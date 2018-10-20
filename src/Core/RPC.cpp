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
#include "Config.h"
#include <fstream>

void RPC::handleNetworkError(const std::string & msg) const
{
    throw RPCConnectionError(msg);
}

void RPC::handleRPCError(Poco::DynamicStruct error) const
{
    throw RPCGeneralError(error["code"].toString(), error["message"].toString());
}

Poco::DynamicStruct RPC::getDataFromRPC(const std::string & method, const Poco::DynamicStruct & args, int id) const
{
    assert(port > 0);

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
        Poco::Net::HTTPClientSession s(GlobalConfig.RPC.hostname, port);
        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, GlobalConfig.RPC.json_uri);
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

RPC::RPC() : port(0)
{

}

RPC::RPC(const RPC& obj)
{
    this->port = obj.port;
}

void RPC::open(unsigned short _port)
{
    port = _port;
}

BalanceRet RPC::getBalance(int id) const
/* Global Wallet Balance */
{
    BalanceRet ret;
    auto json = getDataFromRPC(RPC_METHOD_GET_BALANCE, {}, id);

    // Ensure RPC is happy.
    if (!json.size()) throw RPCGeneralError("-1", "JSON Result is size 0");
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

std::string RPC::getAddress(int id) const
{
    auto json = getDataFromRPC(RPC_METHOD_GET_ADDRESS, {}, id);

    // Ensure RPC is happy.
    if (!json.size()) throw RPCGeneralError("-1", "JSON Result is size 0");
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

unsigned int RPC::getBlockHeight(int id) const
{
    auto json = getDataFromRPC(RPC_METHOD_GET_BLK_HEIGHT, {}, id);

    // Ensure RPC is happy.
    if (!json.size()) throw RPCGeneralError("-1", "JSON Result is size 0");
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

TransferRet RPC::tranfer(std::uint64_t payment_id, std::uint64_t amount, const std::string & address, int id)  const
{
    return tranfer(Poco::format("%064Lu", payment_id), amount, address, id);
}

TransferRet RPC::sweepAll(std::uint64_t payment_id, const std::string & address, int id) const
{
    return sweepAll(Poco::format("%064Lu", payment_id), address, id);
}

TransferRet  RPC::tranfer(const std::string & payment_id, std::uint64_t amount, const std::string & address, int id) const
{
    TransferRet ret;

    // Building JSON string
    Poco::DynamicStruct params;
    Poco::Dynamic::Array destinations;

    Poco::DynamicStruct object;
    object["amount"] = amount;
    object["address"] = address;
    destinations.push_back(object);

    if (address.length() != GlobalConfig.RPC.integrated_address_length)
        params["payment_id"] = payment_id;

    params["destinations"] = destinations;
    params["mixin"] = GlobalConfig.RPC.mixin;
    params["get_tx_key"] = true;
    params["unlock_time"] = 0;

    auto json = getDataFromRPC(RPC_METHOD_TRANSFER, params, id);

    // Ensure RPC is happy.
    if (!json.size()) throw RPCGeneralError("-1", "JSON Result is size 0");
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

TransferRet  RPC::sweepAll(const std::string & payment_id, const std::string & address, int id) const
{
    TransferRet ret;

    // Building JSON string
    Poco::DynamicStruct params;

    params["address"] = address;

    if (address.length() != GlobalConfig.RPC.integrated_address_length)
        params["payment_id"] = payment_id;

    params["mixin"] = GlobalConfig.RPC.mixin;
    params["get_tx_keys"] = true;
    params["unlock_time"] = 0;

    auto json = getDataFromRPC(RPC_METHOD_SWEEP_ALL, params, id);

    // Ensure RPC is happy.
    if (!json.size()) throw RPCGeneralError("-1", "JSON Result is size 0");
    if (!json["error"].isEmpty())
    {
        handleRPCError(json["error"].extract<Poco::DynamicStruct>());
        return {};
    }

    auto result = json["result"].extract<Poco::DynamicStruct>();

    auto tx_hash_list = result["tx_hash_list"].extract<Poco::Dynamic::Array>();
    auto tx_key_list = result["tx_key_list"].extract<Poco::Dynamic::Array>();

    ret.fee = 0;
    ret.tx_hash = tx_hash_list[0].toString();
    ret.tx_key = tx_key_list[0].toString();

    return ret;
}

TransferList RPC::getTransfers(int id) const
{
    struct TransferList incomingTransactions;

    Poco::DynamicStruct params;
    params["in"] = true;
    params["out"] = true;
    auto json = getDataFromRPC(RPC_METHOD_GET_TRANSFERS, params, id);

    // Ensure RPC is happy.
    if (!json.size()) throw RPCGeneralError("-1", "JSON Result is size 0");
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
                // ** Bug fix: If the user sent a non numeric payment id the RPCManager would fail processing new txs.
                try
                {
                    ts.payment_id = it["payment_id"].convert<std::uint64_t>();
                }
                catch (const Poco::Exception & exp)
                {
                    ts.payment_id = 0;
                }
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

bool RPC::createWallet(const std::string & name, const std::string & password, const std::string & language, int id) const
{
    // Ensure we dont overwrite a wallet.
    if (Util::doesWalletExist(GlobalConfig.RPC.wallet_path + name)) return false;

    Poco::DynamicStruct data;

    data["filename"] = name;
    data["password"] = password;
    data["language"] = language;

    auto json = getDataFromRPC(RPC_METHOD_CREATE_WALLET, data, id);

    // Ensure RPC is happy.
    if (!json.size()) throw RPCGeneralError("-1", "JSON Result is size 0");
    if (!json["error"].isEmpty())
    {
        handleRPCError(json["error"].extract<Poco::DynamicStruct>());
        return false;
    }

    // Monero Issue #3315 (https://github.com/monero-project/monero/pull/3315)
    // Monero removed the automation creation of wallet.address.txt files
    // Which is required for TIPBOT to function securely.

    // Therefore, this code is to create that if the RPC doesn't.
    const auto addressPathStr = GlobalConfig.RPC.wallet_path + name + ".address.txt";
    if (!Util::doesWalletExist(addressPathStr))
    {
        // Create Address .txt
        const auto myAddress = this->getAddress();

        std::ofstream addressOut(addressPathStr, std::ios::trunc);

        if (addressOut.is_open())
            addressOut << myAddress;
        else
            throw RPCGeneralError("-1", "Could not create wallet.address.txt file. This wallet will not function correctly!");

        addressOut.close();
    }

    // Ensure Wallet Exists
    return Util::doesWalletExist(GlobalConfig.RPC.wallet_path + name);
}

bool RPC::openWallet(const std::string & name, const std::string & password, int id) const
{
    // Ensure Wallet Exists
    if (!Util::doesWalletExist(GlobalConfig.RPC.wallet_path + name))
        createWallet(name, password);

    Poco::DynamicStruct data;

    data["filename"] = name;
    data["password"] = password;

    auto json = getDataFromRPC(RPC_METHOD_OPEN_WALLET, data, id);

    if (!json.size()) throw RPCGeneralError("-1", "JSON Result is size 0");
    if (!json["error"].isEmpty())
    {
        handleRPCError(json["error"].extract<Poco::DynamicStruct>());
        return false;
    }

    return true;
}

void RPC::stopWallet(int id) const
// This doesn't close the wallet but the RPC.
{
    auto json = getDataFromRPC(RPC_METHOD_CLOSE_RPC, {}, id);

    if (!json.size()) throw RPCGeneralError("-1", "JSON Result is size 0");
    if (!json["error"].isEmpty())
    {
        handleRPCError(json["error"].extract<Poco::DynamicStruct>());
    }
}

void RPC::store(int id) const
{
    auto json = getDataFromRPC(RPC_METHOD_STORE, {}, id);

    if (!json.size()) throw RPCGeneralError("-1", "JSON Result is size 0");
    if (!json["error"].isEmpty())
    {
        handleRPCError(json["error"].extract<Poco::DynamicStruct>());
    }
}

void RPC::rescanSpent(int id) const
{
    auto json = getDataFromRPC(RPC_METHOD_RESCAN_SPENT, {}, id);

    if (!json["error"].isEmpty())
    {
        handleRPCError(json["error"].extract<Poco::DynamicStruct>());
    }
}

void RPC::setTXNote(const std::vector<std::string> & txVect, const std::vector<std::string> & noteVect, int id) const
{
    Poco::DynamicStruct params;
    Poco::Dynamic::Array TX_ARR(txVect.begin(), txVect.end());
    Poco::Dynamic::Array NOTE_ARR(noteVect.begin(), noteVect.end());
    params["txids"] = TX_ARR;
    params["notes"] = NOTE_ARR;

    auto json = getDataFromRPC(RPC_METHOD_SET_TX_NOTE, params, id);

    if (!json.size()) throw RPCGeneralError("-1", "JSON Result is size 0");
    if (!json["error"].isEmpty())
    {
        handleRPCError(json["error"].extract<Poco::DynamicStruct>());
    }
}

std::vector<std::string> RPC::getTXNote(const std::vector<std::string> & txVect, int id) const
{
    Poco::DynamicStruct params;
    Poco::Dynamic::Array TX_ARR(txVect.begin(), txVect.end());
    params["txids"] = TX_ARR;
    auto json = getDataFromRPC(RPC_METHOD_GET_TX_NOTE, params, id);

    if (!json.size()) throw RPCGeneralError("-1", "JSON Result is size 0");
    if (!json["error"].isEmpty())
    {
        handleRPCError(json["error"].extract<Poco::DynamicStruct>());
    }

    auto result = json["result"]["notes"].extract<Poco::Dynamic::Array>();

    std::vector<std::string> ret;
    for (auto it : result)
    {
        ret.emplace_back(it.toString());
    }
    return ret;
}

unsigned short RPC::getPort() const
{
    return port;
}

RPC& RPC::operator=(const RPC &rhs)
{
    if (this != &rhs) {
        this->port = rhs.port;
    }
    return *this;
}
