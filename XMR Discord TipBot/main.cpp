#include "Account.h"
#include "RPCException.h"
#include <iostream>
#include "sleepy_discord/websocketpp_websocket.h"

class myClientClass : public SleepyDiscord::DiscordClient {
public:
	using SleepyDiscord::DiscordClient::DiscordClient;
	void onMessage(SleepyDiscord::Message message) {
		if (message.startsWith("hello"))
			sendMessage(message.channelID, "Sup " + message.author.username);
	}
};

int main()
{
	myClientClass client("token", 2);
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