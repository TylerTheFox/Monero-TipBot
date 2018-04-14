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
#pragma once
#include <map>
#include <stack>
#include "types.h"
#include "RPC.h"
#include "Poco/Timestamp.h"
#include "Account.h"
#include "Poco/Runnable.h"
#include "Discord.h"

#define						STARTING_PORT_NUMBER	10000
#define						MAX_RPC_LIMIT			100

class ITNS_TIPBOT;

struct RPCProc
{
	RPCProc() : myID(0), pid(0) {}
	RPCProc(const RPCProc & obj)
	{
		*this = obj;
	}

	DiscordID											myID;
	Poco::Timestamp										timestamp;
	unsigned int										pid;
	RPC													MyRPC;
	Account												MyAccount;
	struct TransferList									Transactions;
	SleepyDiscord::Snowflake<SleepyDiscord::Channel>	ChannelID;

	RPCProc& operator=(const RPCProc & obj)
	{
		myID = obj.myID;
		timestamp = obj.timestamp;
		pid = obj.pid;
		MyRPC = obj.MyRPC;
		MyAccount = obj.MyAccount;
		Transactions = obj.Transactions;
		ChannelID = obj.ChannelID;
		return *this;
	}
};

// Mutex only needed for map and stack add/remove.
class RPCManager : public Poco::Runnable
{
public:
	RPCManager();
	~RPCManager();

	void									setDiscordPtr(ITNS_TIPBOT* ptr);
	time_t									getTimeStarted(DiscordID id);
	Account &								getAccount(DiscordID id);

	virtual void							run();
	void									processNewTransactions();

	static const RPC&						getGlobalBotRPC();
private:
	Poco::Mutex								mu;
	unsigned short							currPortNum;
	static struct RPCProc*					BotRPCProc;
	std::map<DiscordID, struct RPCProc>		RPCMap;
	ITNS_TIPBOT*							DiscordPtr;

	bool									isRPCRunning(DiscordID id);
	struct RPCProc							SpinUpNewRPC(DiscordID id);
	void									SpinDownRPC(DiscordID id);
	struct RPCProc &						FindOldestRPC();
	void									SaveWallets();
};

extern RPCManager							RPCMan;