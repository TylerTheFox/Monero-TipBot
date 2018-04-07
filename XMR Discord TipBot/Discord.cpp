#include "Discord.h"
#include "Poco/NumberParser.h"
#include "Poco/StringTokenizer.h"
#include "RPCException.h"
#include "DiscordCommands.h"
unsigned long long ITNS_TIPBOT::convertSnowflakeToInt64(SleepyDiscord::Snowflake<SleepyDiscord::User> id)
{
	return Poco::NumberParser::parseUnsigned64(static_cast<std::string>(id));
}

void ITNS_TIPBOT::onMessage(SleepyDiscord::Message message)
{
	DiscordCommands::ProcessCommand(this, message);
}
