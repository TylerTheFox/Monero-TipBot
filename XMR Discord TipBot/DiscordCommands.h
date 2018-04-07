#pragma once
#include "sleepy_discord/websocketpp_websocket.h"

class ITNS_TIPBOT;
struct Command
{
	std::string		name;
	void *			func;
	std::string		params;
};
typedef void(*CommandFunc)(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const Command & me);

namespace DiscordCommands
{
	void ProcessCommand(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message);

	void Help(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me);
	void Balance(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me);
	void MyAddress(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me);
	void History(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me);
	void Withdraw(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me);
	void WithdrawAll(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me);
	void Give(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me);
	void GiveAll(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me);

	void CommandParseError(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message& message, const struct Command & me);
};
