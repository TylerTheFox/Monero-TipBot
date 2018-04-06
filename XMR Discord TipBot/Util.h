#pragma once
#include <string>
#include "Poco/Process.h"
#include "Poco/Pipe.h"

class Util
{
public:
	Util() = delete;
	~Util() = delete;

	static void start_RPC();
	static void stop_RPC();

	static bool doesWalletExist(const std::string & name);
private:
	static bool			RPC_RUNNING;
	static Poco::Pipe	rpc_pipe;
	static int			rpc_pid;
};