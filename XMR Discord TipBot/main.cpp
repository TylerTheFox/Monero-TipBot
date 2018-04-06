#include "Account.h"
#include "RPCException.h"
#include <iostream>
#include "Poco/NumberParser.h"
#include "sleepy_discord/websocketpp_websocket.h"

class ITNS_TIPBOT : public SleepyDiscord::DiscordClient {
public:
	using SleepyDiscord::DiscordClient::DiscordClient;

	static unsigned long long convertSnowflakeToInt64(SleepyDiscord::Snowflake<SleepyDiscord::User> id)
	{
		return Poco::NumberParser::parseUnsigned64(static_cast<std::string>(id));
	}

	void onMessage(SleepyDiscord::Message message) {
		try
		{

			if (message.content == "!balance")
			{
				Account usr(convertSnowflakeToInt64(message.author.ID));
				sendMessage(message.channelID, Poco::format("%s#%s: Your Balance is %Lu ITNS and your Unlocked Balance is %Lu ITNS", message.author.username, message.author.discriminator, usr.getBalance(), usr.getUnlockedBalance()));
			} else if (message.content == "!myaddress")
			{
				Account usr(convertSnowflakeToInt64(message.author.ID));
				sendMessage(message.channelID, Poco::format("%s#%s: Your ITNS Address is: %s", message.author.username, message.author.discriminator, usr.getMyAddress()));
			}

			if (message.startsWith("hello"))
				sendMessage(message.channelID, "Sup " + message.author.username);

		}
		catch (AppGeneralException & exp)
		{ 
			sendMessage(message.channelID, std::string(exp.what()) + " --- " + exp.getGeneralError());
		}
	}
};

int main()
{
	ITNS_TIPBOT client("DISCORD BOT TOKEN GOES HERE", 2);
	client.run();

	try
	{
		Account Discord_User(206430811430322176/* Brandan */);
		std::cout << "My Address: " << Discord_User.getMyAddress() << "\nCurrent Balance: " << Discord_User.getBalance() << "\nCurrent Unlocked Balance: " << Discord_User.getUnlockedBalance() << '\n';
		auto a = Discord_User.transferMoneytoAnotherDiscordUser(0, 9781);
		a = Discord_User.transferMoneyToAddress(2000000000, "");
	}
	catch (AppGeneralException & exp)
	{
		std::cerr << exp.what() << " -- " << exp.getGeneralError();
	}
}