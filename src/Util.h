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
#include <string>
#include "Poco/Process.h"
#include "Poco/Pipe.h"

#define DISCORD_WALLET_MASK "Discord-User-%Lu"

class Util
{
public:
	Util() = delete;
	~Util() = delete;

	static void start_RPC();
	static void stop_RPC();

	static bool doesWalletExist(const std::string & name);
	static bool doesWalletExist(std::uint64_t DIS_ID);

	static std::string getWalletStrFromIID(std::uint64_t DIS_ID);

private:
	static bool			RPC_RUNNING;
	static Poco::Pipe	rpc_pipe;
	static int			rpc_pid;
};