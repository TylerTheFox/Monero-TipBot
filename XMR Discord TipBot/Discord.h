#pragma once
#include "sleepy_discord/websocketpp_websocket.h"

class ITNS_TIPBOT : public SleepyDiscord::DiscordClient {
public:
	using SleepyDiscord::DiscordClient::DiscordClient;

	static unsigned long long convertSnowflakeToInt64(SleepyDiscord::Snowflake<SleepyDiscord::User> id);
	void onMessage(SleepyDiscord::Message message);

private:
};