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
#include "DiscordCommands.h"
#include "RPCException.h"
#include "Discord.h"
#include <Poco/Exception.h>
#include <Poco/StringTokenizer.h>
#include "Account.h"

const struct Command Commands[] =
{
	{	"!help",			reinterpret_cast<void*>(&DiscordCommands::Help),		""									},
	{	"!balance",		reinterpret_cast<void*>(&DiscordCommands::Balance),	""									},
	{	"!myaddress",		reinterpret_cast<void*>(&DiscordCommands::MyAddress),	""									},
	{	"!history",			reinterpret_cast<void*>(&DiscordCommands::History),	""									},
	{	"!withdraw",		reinterpret_cast<void*>(&DiscordCommands::Withdraw),	"[amount] [address]"				},
	{	"!withdrawall",		reinterpret_cast<void*>(&DiscordCommands::Withdraw),	"[amount]"							},
	{	"!give",			reinterpret_cast<void*>(&DiscordCommands::Give),		"[amount] [@User1 @User2...]"		},
	{	"!giveall",			reinterpret_cast<void*>(&DiscordCommands::GiveAll),	"[@User]"							},
};

void DiscordCommands::ProcessCommand(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message)
{
	try
	{
		Poco::StringTokenizer cmd(message.content, " ");

		if (cmd.count())
		{
			for (const auto & command : Commands)
			{
				if (command.name == cmd[0])
				{
					reinterpret_cast<CommandFunc>(command.func)(DiscordPtr, message, command);
				}
			}
		}
	}
	catch (const Poco::Exception & exp)
	{
		DiscordPtr->sendMessage(message.channelID, "Poco Error: ---" + std::string(exp.what()));
	}
	catch (AppGeneralException & exp)
	{
		DiscordPtr->sendMessage(message.channelID, std::string(exp.what()) + " --- " + exp.getGeneralError());
	}
}

void DiscordCommands::Help(ITNS_TIPBOT* DiscordPtr, const SleepyDiscord::Message& message, const Command& me)
{
	std::stringstream ss;

	ss << "ITNS Bot Commands:\\n";

	ss << "```";
	for (auto cmd : Commands)
	{
		ss << cmd.name << " " << cmd.params << "\\n";
	}
	ss << "```";

	DiscordPtr->sendMessage(message.channelID, ss.str());
}

void DiscordCommands::Balance(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message& message, const struct Command & me)
{
	Account usr(DiscordPtr->convertSnowflakeToInt64(message.author.ID));
	DiscordPtr->sendMessage(message.channelID, Poco::format("%s#%s: Your Balance is %f ITNS and your Unlocked Balance is %f ITNS", message.author.username, message.author.discriminator, usr.getBalance() / ITNS_OFFSET, usr.getUnlockedBalance() / ITNS_OFFSET));
}

void DiscordCommands::MyAddress(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message& message, const struct Command & me)
{
	Account usr(DiscordPtr->convertSnowflakeToInt64(message.author.ID));
	DiscordPtr->sendMessage(message.channelID, Poco::format("%s#%s: Your ITNS Address is: %s", message.author.username, message.author.discriminator, usr.getMyAddress()));
}

void DiscordCommands::History(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message& message, const struct Command & me)
{
	Account usr(DiscordPtr->convertSnowflakeToInt64(message.author.ID));
	const auto trxs = usr.getTransactions();

	std::stringstream ss;

	const auto addtoss = [&ss](const std::set<struct TransferItem, TransferItemCmp> & sset)
	{
		auto i = 0;
		ss << "```Amount | Payment ID | Block Height | TX Hash\\n";

		for (auto tx : sset)
		{
			if (i == 5) break;
			ss << tx.amount / ITNS_OFFSET << " | " << tx.payment_id << " | " << tx.block_height << " | " << tx.tx_hash << "\\n";
			i++;
		}
		ss << "```";
	};

	ss << "Your Incoming Transactions (last 5): \\n";
	addtoss(trxs.tx_in);

	ss << "\\nYour Outgoing Transactions (last 5): \\n";
	addtoss(trxs.tx_out);

	DiscordPtr->sendMessage(message.channelID, ss.str());
}

void DiscordCommands::Withdraw(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message& message, const struct Command & me)
{
	Poco::StringTokenizer cmd(message.content, " ");
	Account usr(DiscordPtr->convertSnowflakeToInt64(message.author.ID));

	if (cmd.count() != 3)
		CommandParseError(DiscordPtr, message, me);
	else
	{
		const auto amount = Poco::NumberParser::parseFloat(cmd[1]);
		const auto& address = cmd[2];
		const auto tx = usr.transferMoneyToAddress(static_cast<std::uint64_t>(amount * ITNS_OFFSET), address);
		DiscordPtr->sendMessage(message.channelID, Poco::format("%s#%s: Withdraw Complete Sent %f ITNS with TX Hash: %s :smiley:", message.author.username, message.author.discriminator, amount, tx.tx_hash));
	}
}

void DiscordCommands::WithdrawAll(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message& message, const struct Command & me)
{
	Poco::StringTokenizer cmd(message.content, " ");
	Account usr(DiscordPtr->convertSnowflakeToInt64(message.author.ID));

	if (cmd.count() != 2)
		CommandParseError(DiscordPtr, message, me);
	else
	{
		const auto& address = cmd[1];
		const auto tx = usr.transferAllMoneyToAddress(address);
		DiscordPtr->sendMessage(message.channelID, Poco::format("%s#%s: Withdraw Complete Sent %f ITNS with TX Hash: %s :smiley:", message.author.username, message.author.discriminator, usr.getBalance() / ITNS_OFFSET, tx.tx_hash));
	}

}

void DiscordCommands::Give(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message& message, const struct Command & me)
{
	Poco::StringTokenizer cmd(message.content, " ");
	Account usr(DiscordPtr->convertSnowflakeToInt64(message.author.ID));

	if (cmd.count() < 2 || message.mentions.empty())
		CommandParseError(DiscordPtr, message, me);
	else
	{
		const auto amount = Poco::NumberParser::parseFloat(cmd[1]);
		for (const auto& user : message.mentions)
		{
			const auto tx = usr.transferMoneytoAnotherDiscordUser(static_cast<std::uint64_t>(amount * ITNS_OFFSET), DiscordPtr->convertSnowflakeToInt64(user.ID));
			DiscordPtr->sendMessage(message.channelID, Poco::format("%s#%s: Giving %f ITNS to %s with TX Hash: %s :smiley:", message.author.username, message.author.discriminator, amount, user.username, tx.tx_hash));
		}
	}

}

void DiscordCommands::GiveAll(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message& message, const struct Command & me)
{
	Poco::StringTokenizer cmd(message.content, " ");
	Account usr(DiscordPtr->convertSnowflakeToInt64(message.author.ID));

	if (cmd.count() != 2 || message.mentions.empty())
		CommandParseError(DiscordPtr, message, me);
	else
	{
		const auto tx = usr.transferAllMoneytoAnotherDiscordUser(DiscordPtr->convertSnowflakeToInt64(message.mentions[0].ID));
		DiscordPtr->sendMessage(message.channelID, Poco::format("%s#%s: Giving %f ITNS to %s with TX Hash: %s :smiley:", message.author.username, message.author.discriminator, static_cast<double>(usr.getBalance() / ITNS_OFFSET), message.mentions[0].username, tx.tx_hash));
	}
}

void DiscordCommands::CommandParseError(ITNS_TIPBOT* DiscordPtr, const SleepyDiscord::Message& message, const Command& me)
{
	DiscordPtr->sendMessage(message.channelID, Poco::format("Command Error --- Correct Usage: !%s %s", me.name, me.params));
}
