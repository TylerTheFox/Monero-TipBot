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
#include "Faucet.h"
#include "Discord.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/Timestamp.h"
#include "Poco/Timespan.h"
#include "RPCManager.h"


#define CLASS_RESOLUTION(x) std::bind(&Faucet::x, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
Faucet::Faucet()
{
    Commands =
    {
        // User Commands 
        // Command            Function                                      Params                              Wallet  Admin   Allowed Channel
        { "!faucet",          CLASS_RESOLUTION(help),                       "",                                 false,  false,  AllowChannelTypes::Any },
        { "!take",            CLASS_RESOLUTION(take),                       "",                                 false,  false,  AllowChannelTypes::Any },
        { "!status",          CLASS_RESOLUTION(status),                     "",                                 false,  true,   AllowChannelTypes::Private },
    };
}

void Faucet::setAccount(Account*)
{
    // Do nothing, we construct this parameter since its static.
}

iterator Faucet::begin()
{
    return Commands.begin();
}

const_iterator Faucet::begin() const
{
    return Commands.begin();
}

const_iterator Faucet::cbegin() const
{
    return Commands.cbegin();
}

iterator Faucet::end()
{
    return Commands.end();
}

const_iterator Faucet::end() const
{
    return Commands.end();

}

const_iterator Faucet::cend() const
{
    return Commands.cend();

}

void Faucet::help(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me)
{
    const auto channelType = DiscordPtr->getDiscordChannelType(message.channelID);
    const auto helpStr = ITNS_TIPBOT::generateHelpText("ITNS Bot Faucet Commands (use ``!give [amount] @Tip Bot`` to donate to faucet):\\n", Commands, channelType, message);
    DiscordPtr->sendMessage(message.channelID, helpStr);
}

void Faucet::take(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me)
{
    auto & myAccountPtr = RPCManager::getGlobalBotAccount();

    myAccountPtr.resyncAccount();

    std::stringstream ss;

    ss << "```";

    const auto & user = DiscordPtr->findUser(ITNS_TIPBOT::convertSnowflakeToInt64(message.author.ID));
    const Poco::Timestamp   current;
    const Poco::Timestamp   joints(user.join_epoch_time);
    const Poco::Timestamp   faucetts(user.faucet_epoch_time);
    const Poco::Timespan    faucettimediff(current - faucetts);
    const Poco::Timespan    jointimediff(current - joints);

    if (jointimediff.days() > MIN_DISCORD_ACCOUNT_IN_DAYS)
    {
        if (faucettimediff.hours() > FAUCET_TIMEOUT)
        {
            if (myAccountPtr.getUnlockedBalance() > 0)
            {
                const auto amount = static_cast<std::uint64_t>(myAccountPtr.getUnlockedBalance()*FAUCET_PERCENTAGE_ALLOWANCE);
                const auto tx = myAccountPtr.transferMoneyToAddress(amount, Account::getWalletAddress(ITNS_TIPBOT::convertSnowflakeToInt64(message.author.ID)));
                user.total_faucet_itns_sent += amount / ITNS_OFFSET;
                ss << Poco::format("%s#%s: You have been granted %0.8f ITNS with TX Hash: %s :smiley:\\n", message.author.username, message.author.discriminator, amount / ITNS_OFFSET, tx.tx_hash);
                user.faucet_epoch_time = current.epochMicroseconds();
                DiscordPtr->saveUserList();
            }
            else ss << "Bot is either broke or has pending transactions, try again later. :disappointed_relieved: \\n";
        }
        else ss << "Too soon, once every " << FAUCET_TIMEOUT << " hours...\\n";
    }
    else ss << "Your Discord account must be older than 7 days\\n";

    ss << " ```";

    DiscordPtr->sendMessage(message.channelID, ss.str());
}

void Faucet::status(ITNS_TIPBOT* DiscordPtr, const SleepyDiscord::Message& message, const Command& me)
{
    auto & myAccountPtr = RPCManager::getGlobalBotAccount();
    const auto & user = DiscordPtr->findUser(ITNS_TIPBOT::convertSnowflakeToInt64(message.author.ID));

    std::stringstream ss;
    ss.precision(8);
    myAccountPtr.resyncAccount();

    ss << "```";
    ss << "Your name is: " << user.username << "\\n";
    ss << "Your ID is: " << user.id << "\\n";
    ss << "Bot current unlocked balance is: " << myAccountPtr.getUnlockedBalance() / ITNS_OFFSET << "\\n";
    ss << "Bot current address is: " << myAccountPtr.getMyAddress() << "\\n";
    ss << "Bot timeout is: " << FAUCET_TIMEOUT << " hours\\n";
    ss << "Minimum Discord Account: " << MIN_DISCORD_ACCOUNT_IN_DAYS << " days\\n";
    ss << "Current Award: " << (myAccountPtr.getUnlockedBalance()*FAUCET_PERCENTAGE_ALLOWANCE) / ITNS_OFFSET << "\\n";
    ss << "Current payout percentage: " << FAUCET_PERCENTAGE_ALLOWANCE*100 << "%\\n";
    ss << "Current Amount Awarded: " << DiscordPtr->totalFaucetAmount() << "\\n";
    ss << "```";

    DiscordPtr->sendMessage(message.channelID, ss.str());

}
