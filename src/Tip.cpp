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
#include "Tip.h"
#include "RPCException.h"
#include "Discord.h"
#include <Poco/StringTokenizer.h>
#include "RPCManager.h"
#include "Config.h"

#define CLASS_RESOLUTION(x) std::bind(&Tip::x, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
Tip::Tip() : MyAccount(nullptr)
{
    globalSettings = {
        true,
        true
    };
    Commands =
    {
        // User Commands 
        // Command            Function                                       Params                              Wallet  Admin   Allowed Channel
        { "!about",           CLASS_RESOLUTION(About),                       "",                                 false,  false,  AllowChannelTypes::Any },
        { "!help",            CLASS_RESOLUTION(Help),                        "",                                 false,  false,  AllowChannelTypes::Any },
        { "!myaddress",       CLASS_RESOLUTION(MyAddress),                   "",                                 false,  false,  AllowChannelTypes::Private },
        { "!blockheight",     CLASS_RESOLUTION(BlockHeight),                 "",                                 true,   false,  AllowChannelTypes::Any },
        { "!balance",         CLASS_RESOLUTION(Balance),                     "",                                 true,   false,  AllowChannelTypes::Any },
        { "!history",         CLASS_RESOLUTION(History),                     "",                                 true,   false,  AllowChannelTypes::Private },
        { "!withdraw",        CLASS_RESOLUTION(Withdraw),                    "[amount] [address]",               true,   false,  AllowChannelTypes::Private },
        { "!withdrawall",     CLASS_RESOLUTION(WithdrawAll),                 "[address]"    ,                    true,   false,  AllowChannelTypes::Private },
        { "!give",            CLASS_RESOLUTION(Give),                        "[amount] [@User1 @User2...]",      true,   false,  AllowChannelTypes::Public },
        { "!giveall",         CLASS_RESOLUTION(GiveAll),                     "[@User]",                          true,   false,  AllowChannelTypes::Public },
        { "!tip",             CLASS_RESOLUTION(Give),                        "[amount] [@User1 @User2...]",      true,   false,  AllowChannelTypes::Public },
        { "!tipall",          CLASS_RESOLUTION(GiveAll),                     "[@User]",                          true,   false,  AllowChannelTypes::Public },
        { "!restartwallet",   CLASS_RESOLUTION(RestartWallet),               "",                                 true,   false,  AllowChannelTypes::Any },

        // Admin
        // Command            Function                                       Params                              Wallet  Admin   Allowed Channel
        { "!togglewithdraw",  CLASS_RESOLUTION(ToggleWithdraw),              "",                                 false,  true,   AllowChannelTypes::Private },
        { "!togglegive",      CLASS_RESOLUTION(ToggleGive),                  "",                                 false,  true,   AllowChannelTypes::Private },
        { "!rescanallwallets",CLASS_RESOLUTION(RescanAllWallets),            "",                                 false,  true,   AllowChannelTypes::Private },
        { "!totalbalance",    CLASS_RESOLUTION(TotalBalance),                "",                                 false,  true,   AllowChannelTypes::Private },
        { "!savewallets",     CLASS_RESOLUTION(SaveWallets),                 "",                                 false,  true,   AllowChannelTypes::Private },
        { "!restartfaucet",   CLASS_RESOLUTION(RestartFaucetWallet),         "",                                 false,  true,   AllowChannelTypes::Private },
        { "!softrestart",     CLASS_RESOLUTION(SoftRestartBot),              "",                                 false,  true,   AllowChannelTypes::Private },
        { "!shutdown",        CLASS_RESOLUTION(Shutdown),                    "",                                 false,  true,   AllowChannelTypes::Private },
        { "!rpcstatus",       CLASS_RESOLUTION(RPCStatus),                   "",                                 false,  true,   AllowChannelTypes::Private },
    };
}

void Tip::save()
{
}

void Tip::load()
{
}

void Tip::Help(TIPBOT* DiscordPtr, const SleepyDiscord::Message& message, const Command& me)
{
    const auto channelType = DiscordPtr->getDiscordChannelType(message.channelID);
    const auto helpStr = TIPBOT::generateHelpText("TipBot Commands:\\n", Commands, channelType, message);
    DiscordPtr->sendMessage(message.channelID, helpStr);
}

void Tip::Balance(TIPBOT * DiscordPtr, const SleepyDiscord::Message& message, const struct Command & me)
{
    DiscordPtr->sendMessage(message.channelID, Poco::format("%s#%s: Your Balance is %0.8f %s and your Unlocked Balance is %0.8f %s", message.author.username, message.author.discriminator, MyAccount->getBalance() / GlobalConfig.RPC.coin_offset, GlobalConfig.RPC.coin_abbv, MyAccount->getUnlockedBalance() / GlobalConfig.RPC.coin_offset, GlobalConfig.RPC.coin_abbv));
}

void Tip::MyAddress(TIPBOT * DiscordPtr, const SleepyDiscord::Message& message, const struct Command & me)
{
    DiscordPtr->sendMessage(message.channelID, Poco::format("%s#%s: Your %s Address is: %s", message.author.username, message.author.discriminator, GlobalConfig.RPC.coin_abbv, Account::getWalletAddress(TIPBOT::convertSnowflakeToInt64(message.author.ID))));
}

void Tip::History(TIPBOT * DiscordPtr, const SleepyDiscord::Message& message, const struct Command & me)
{
    const auto trxs = MyAccount->getTransactions();

    std::stringstream ss;

    const auto addtoss = [&ss, DiscordPtr](const std::multiset<struct TransferItem, TransferItemCmp> & sset)
    {
        auto i = 0;
        ss << "```Amount | User | Block Height | TX Hash\\n";

        for (auto tx : sset)
        {
            if (i == 5) break;
            ss << tx.amount / GlobalConfig.RPC.coin_offset << " | " << DiscordPtr->findUser(tx.payment_id).username << " | " << tx.block_height << " | " << tx.tx_hash << "\\n";
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

void Tip::Withdraw(TIPBOT * DiscordPtr, const SleepyDiscord::Message& message, const struct Command & me)
{
    if (globalSettings.withdrawAllowed)
    {
        Poco::StringTokenizer cmd(message.content, " ");

        if (cmd.count() != 3)
            DiscordPtr->CommandParseError(message, me);
        else
        {
            const auto amount = Poco::NumberParser::parseFloat(cmd[1]);
            const auto& address = cmd[2];
            const auto tx = MyAccount->transferMoneyToAddress(static_cast<std::uint64_t>(amount * GlobalConfig.RPC.coin_offset), address);
            DiscordPtr->sendMessage(message.channelID, Poco::format("%s#%s: Withdraw Complete Sent %0.8f %s with TX Hash: %s :smiley:", message.author.username, message.author.discriminator, amount, GlobalConfig.RPC.coin_abbv, tx.tx_hash));
        }
    }
    else
    {
        DiscordPtr->sendMessage(message.channelID, Poco::format("%s#%s: Withdraws aren't allowed at the moment.", message.author.username, message.author.discriminator));
    }
}

void Tip::WithdrawAll(TIPBOT * DiscordPtr, const SleepyDiscord::Message& message, const struct Command & me)
{
    if (globalSettings.withdrawAllowed)
    {
        Poco::StringTokenizer cmd(message.content, " ");

        if (cmd.count() != 2)
            DiscordPtr->CommandParseError(message, me);
        else
        {
            const auto& address = cmd[1];
            const auto tx = MyAccount->transferAllMoneyToAddress(address);
            DiscordPtr->sendMessage(message.channelID, Poco::format("%s#%s: Withdraw Complete Sent %0.8f %s with TX Hash: %s :smiley:", message.author.username, message.author.discriminator, MyAccount->getUnlockedBalance() / GlobalConfig.RPC.coin_offset, GlobalConfig.RPC.coin_abbv, tx.tx_hash));
        }
    }
    else
    {
        DiscordPtr->sendMessage(message.channelID, Poco::format("%s#%s: Withdraws aren't allowed at the moment. :cold_sweat:", message.author.username, message.author.discriminator));
    }
}

void Tip::Give(TIPBOT * DiscordPtr, const SleepyDiscord::Message& message, const struct Command & me)
{
    if (globalSettings.giveAllowed)
    {
        Poco::StringTokenizer cmd(message.content, " ");

        if (cmd.count() < 2 || message.mentions.empty())
            DiscordPtr->CommandParseError(message, me);
        else
        {
            const auto amount = Poco::NumberParser::parseFloat(cmd[1]);
            for (const auto& user : message.mentions)
            {
                const auto tx = MyAccount->transferMoneytoAnotherDiscordUser(static_cast<std::uint64_t>(amount * GlobalConfig.RPC.coin_offset), DiscordPtr->convertSnowflakeToInt64(user.ID));
                DiscordPtr->sendMessage(message.channelID, Poco::format("%s#%s: Giving %0.8f %s to %s with TX Hash: %s :smiley:", message.author.username, message.author.discriminator, amount, GlobalConfig.RPC.coin_abbv, user.username, tx.tx_hash));
            }
        }
    }
    else
    {
        DiscordPtr->sendMessage(message.channelID, Poco::format("%s#%s: Give isn't allowed at the moment. :cold_sweat:", message.author.username, message.author.discriminator));
    }
}

void Tip::GiveAll(TIPBOT * DiscordPtr, const SleepyDiscord::Message& message, const struct Command & me)
{
    if (globalSettings.giveAllowed)
    {
        Poco::StringTokenizer cmd(message.content, " ");

        if (cmd.count() != 2 || message.mentions.empty())
            DiscordPtr->CommandParseError(message, me);
        else
        {
            const auto tx = MyAccount->transferAllMoneytoAnotherDiscordUser(DiscordPtr->convertSnowflakeToInt64(message.mentions[0].ID));
            DiscordPtr->sendMessage(message.channelID, Poco::format("%s#%s: Giving %0.8f %s to %s with TX Hash: %s :smiley:", message.author.username, message.author.discriminator, static_cast<double>(MyAccount->getUnlockedBalance() / GlobalConfig.RPC.coin_offset), GlobalConfig.RPC.coin_abbv, message.mentions[0].username, tx.tx_hash));
        }
    }
    else
    {
        DiscordPtr->sendMessage(message.channelID, Poco::format("%s#%s: Give isn't allowed at the moment. :cold_sweat:", message.author.username, message.author.discriminator));
    }
}

void Tip::About(TIPBOT* DiscordPtr, const SleepyDiscord::Message& message, const Command& me)
{
    DiscordPtr->sendMessage(message.channelID, Poco::format(aboutStr, GlobalConfig.About.major, GlobalConfig.About.minor));
}

void Tip::BlockHeight(TIPBOT* DiscordPtr, const SleepyDiscord::Message& message, const Command& me)
{
    DiscordPtr->sendMessage(message.channelID, Poco::format("Your wallet's current block height is: %?i", MyAccount->getBlockHeight()));
}

void Tip::RestartWallet(TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const Command & me)
{
    RPCMan->restartWallet(MyAccount->getDiscordID());
    DiscordPtr->sendMessage(message.channelID,"Discord Wallet restarted successfully! It may take a minute to resync.");
}

void Tip::ToggleWithdraw(TIPBOT* DiscordPtr, const SleepyDiscord::Message& message, const Command& me)
{
    globalSettings.withdrawAllowed = !globalSettings.withdrawAllowed;
    DiscordPtr->sendMessage(message.channelID, Poco::format("Withdraw Enabled: %b", globalSettings.withdrawAllowed));
}

void Tip::ToggleGive(TIPBOT* DiscordPtr, const SleepyDiscord::Message& message, const Command& me)
{
    globalSettings.giveAllowed = !globalSettings.giveAllowed;
    DiscordPtr->sendMessage(message.channelID, Poco::format("Give Enabled: %b", globalSettings.giveAllowed));
}

void Tip::RescanAllWallets(TIPBOT* DiscordPtr, const SleepyDiscord::Message& message, const Command& me)
{
    RPCMan->rescanAll();
    DiscordPtr->sendMessage(message.channelID, "Rescan spent complete!");
}

void Tip::TotalBalance(TIPBOT* DiscordPtr, const SleepyDiscord::Message& message, const Command& me)
{
    DiscordPtr->sendMessage(message.channelID, Poco::format("I currently manage %0.8f locked %s and %0.8f unlocked %s! (Excluding lottery)", RPCMan->getTotalBalance() / GlobalConfig.RPC.coin_offset, GlobalConfig.RPC.coin_abbv, RPCMan->getTotalUnlockedBalance() / GlobalConfig.RPC.coin_offset, GlobalConfig.RPC.coin_abbv));
}

void Tip::SaveWallets(TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const Command & me)
{
    RPCMan->saveallWallets();   
    DiscordPtr->sendMessage(message.channelID, "Wallets saved!");
}

void Tip::RestartFaucetWallet(TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const Command & me)
{
    RPCMan->restartWallet(RPCMan->getBotDiscordID());
    DiscordPtr->sendMessage(message.channelID, "Faucet Restarted!");
}

iterator Tip::begin()
{
    return Commands.begin();
}

const_iterator Tip::begin() const
{
    return Commands.begin();
}

const_iterator Tip::cbegin() const
{
    return Commands.cbegin();
}

iterator Tip::end()
{
    return Commands.end();
}

const_iterator Tip::end() const
{
    return Commands.end();
}

const_iterator Tip::cend() const
{
    return Commands.cend();
}

void Tip::setAccount(Account* acc)
{
    this->MyAccount = acc;
}

void Tip::SoftRestartBot(TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me)
{
    // Send restart message.
    DiscordPtr->sendMessage(message.channelID, "Restart Command sent!");
    DiscordPtr->quit();
}

void Tip::Shutdown(TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me)
{
    DiscordPtr->sendMessage(message.channelID, "Shutdown Command sent! -- Good bye.");
    GlobalConfig.General.Quitting = true;
    DiscordPtr->quit();
}

void Tip::RPCStatus(TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me)
{
    DiscordPtr->sendMessage(message.channelID, RPCMan->status());
}
