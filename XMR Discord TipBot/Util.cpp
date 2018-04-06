#include "Util.h"
#include "RPC.h"
#include "Poco/File.h"
#include "Poco/Process.h"
#include <assert.h>

bool		Util::RPC_RUNNING	= false;
int			Util::rpc_pid		= 0;
Poco::Pipe	Util::rpc_pipe;

void Util::start_RPC()
{
	if (!RPC_RUNNING)
	{
		RPC_RUNNING = true;

		std::vector<std::string> args;
		args.push_back("--wallet-dir");
		args.push_back(WALLET_PATH);
		args.push_back("--rpc-bind-port");
		args.push_back(Poco::format("%d", RPC_PORT));
		args.push_back("--daemon-address");
		args.push_back(DAEMON_ADDRESS);
		args.push_back("--disable-rpc-login");
		args.push_back("--trusted-daemon");

		Poco::ProcessHandle rpc_handle = Poco::Process::launch(RPC_FILENAME, args, 0, &rpc_pipe, 0);
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
