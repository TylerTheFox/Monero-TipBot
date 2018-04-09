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
#include "Discord.h"
#include "Poco/NumberParser.h"
#include "Poco/StringTokenizer.h"
#include "RPCException.h"
#include "DiscordCommands.h"
std::uint64_t ITNS_TIPBOT::convertSnowflakeToInt64(SleepyDiscord::Snowflake<SleepyDiscord::User> id)
{
	return Poco::NumberParser::parseUnsigned64(static_cast<std::string>(id));
}

void ITNS_TIPBOT::onMessage(SleepyDiscord::Message message)
{
	DiscordCommands::ProcessCommand(this, message);
}
