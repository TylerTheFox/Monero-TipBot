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
#include "Util.h"
#include "RPC.h"
#include "Poco/File.h"
#include "Poco/Process.h"
#include <cassert>

bool		Util::RPC_RUNNING	= false;
int			Util::rpc_pid		= 0;
Poco::Pipe	Util::rpc_pipe;

void Util::start_RPC()
{
	if (!RPC_RUNNING)
	{
		RPC_RUNNING = true;

		std::vector<std::string> args;
		args.emplace_back("--wallet-dir");
		args.emplace_back(WALLET_PATH);
		args.emplace_back("--rpc-bind-port");
		args.push_back(Poco::format("%d", RPC_PORT));
		args.emplace_back("--daemon-address");
		args.emplace_back(DAEMON_ADDRESS);
		args.emplace_back("--disable-rpc-login");
		args.emplace_back("--trusted-daemon");

		Poco::ProcessHandle rpc_handle = Poco::Process::launch(RPC_FILENAME, args, nullptr, &rpc_pipe, nullptr);
		rpc_pid = rpc_handle.id();
		assert(rpc_pid > 0);
	}
}

void Util::stop_RPC()
{
	if (!RPC_RUNNING)
	{
		assert(rpc_pid);
		Poco::Process::requestTermination(rpc_pid);
		rpc_pid = 0;
		RPC_RUNNING = false;
	}
}

bool Util::doesWalletExist(const std::string & name)
{
	return Poco::File(name).exists();
}
