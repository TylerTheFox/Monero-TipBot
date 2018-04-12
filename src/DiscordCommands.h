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
#include "sleepy_discord/websocketpp_websocket.h"

class ITNS_TIPBOT;
struct Command
{
	std::string		name;
	void *			func;
	std::string		params;
	bool			opensWallet;
};
typedef void(*CommandFunc)(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const Command & me);

namespace DiscordCommands
{
	void ProcessCommand(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message);
	void CommandParseError(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message& message, const struct Command & me);

	void Help(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me);
	void Balance(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me);
	void MyAddress(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me);
	void History(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me);
	void Withdraw(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me);
	void WithdrawAll(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me);
	void Give(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me);
	void GiveAll(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me);
	void About(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me);
};
