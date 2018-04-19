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
#include "RPCManager.h"

const DiscordID DiscordAdmins[] =
{
    380370690030829578, // Valiant
    144619872444219392, // ddvs1
    266700783897018369, // SlowGrowth
    415162452725202944, // iedemam
    345699014806732800, // ThePigwee
    206430811430322176, // Brandan
};

const std::string AllowChannelTypeNames[] =
{
    "Public Channel Only",
    "Direct Message Only"
};

const struct Command Commands[] =
{
    // User Commands 
    // Command              Function                                                                Params                              Wallet  Admin   Allowed Channel
    {   "!about",           reinterpret_cast<void*>(&DiscordCommands::About),                       "",                                 false,  false,  AllowChannelTypes::Any                },
    {   "!help",            reinterpret_cast<void*>(&DiscordCommands::Help),                        "",                                 false,  false,  AllowChannelTypes::Any                },
    {   "!myaddress",       reinterpret_cast<void*>(&DiscordCommands::MyAddress),                   "",                                 false,  false,  AllowChannelTypes::Private            },
    {   "!blockheight",     reinterpret_cast<void*>(&DiscordCommands::BlockHeight),                 "",                                 true,   false,  AllowChannelTypes::Any                },
    {   "!balance",         reinterpret_cast<void*>(&DiscordCommands::Balance),                     "",                                 true,   false,  AllowChannelTypes::Any                },
    {   "!history",         reinterpret_cast<void*>(&DiscordCommands::History),                     "",                                 true,   false,  AllowChannelTypes::Private            },
    {   "!withdraw",        reinterpret_cast<void*>(&DiscordCommands::Withdraw),                    "[amount] [address]",               true,   false,  AllowChannelTypes::Private            },
    {   "!withdrawall",     reinterpret_cast<void*>(&DiscordCommands::WithdrawAll),                 "[address]"    ,                    true,   false,  AllowChannelTypes::Private            },
    {   "!give",            reinterpret_cast<void*>(&DiscordCommands::Give),                        "[amount] [@User1 @User2...]",      true,   false,  AllowChannelTypes::Public             },
    {   "!giveall",         reinterpret_cast<void*>(&DiscordCommands::GiveAll),                     "[@User]",                          true,   false,  AllowChannelTypes::Public             },

    // Admin
    // Command              Function                                                                Params                              Wallet  Admin   Allowed Channel
    {   "!togglewithdraw",  reinterpret_cast<void*>(&DiscordCommands::ToggleWithdrawAllowed),       "",                                 false,  true,   AllowChannelTypes::Private            },
    {   "!togglegive",      reinterpret_cast<void*>(&DiscordCommands::ToggleGiveAllowed),           "",                                 false,  true,   AllowChannelTypes::Private            },
    {   "!togglebot",       reinterpret_cast<void*>(&DiscordCommands::ToggleCommandsAllowed),       "",                                 false,  true,   AllowChannelTypes::Private            },
};

struct Settings globalSettings = {
    true,
    true,
    true
};

#define        VERSION_MAJOR 1
#define        VERSION_MINOR 2

const char *aboutStr =
"```ITNS TipBot v%d.%d\\n"
"(C) Brandan Tyler Lasley 2018\\n"
"Github: https://github.com/Brandantl/IntenseCoin-TipBot \\n"
"BTC: 1KsX66J98WMgtSbFA5UZhVDn1iuhN5B6Hm\\n"
"ITNS: iz5ZrkSjiYiCMMzPKY8JANbHuyChEHh8aEVHNCcRa2nFaSKPqKwGCGuUMUMNWRyTNKewpk9vHFTVsHu32X3P8QJD21mfWJogf\\n"
"XMR: 44DudyMoSZ5as1Q9MTV6ydh4BYT6BMCvxNZ8HAgeZo9SatDVixVjZzvRiq9fiTneykievrWjrUvsy2dKciwwoUv15B9MzWS\\n```";

Account*            MyAccount;

bool isCommandAllowedToBeExecuted(ITNS_TIPBOT* DiscordPtr, const SleepyDiscord::Message & message, const Command& command, int channelType)
{
    return !command.adminTools || (command.adminTools && channelType == AllowChannelTypes::Private && DiscordCommands::isUserAdmin(DiscordPtr, message));
}

void DiscordCommands::ProcessCommand(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message)
{
    int channelType;
    try
    {
        Poco::StringTokenizer cmd(message.content, " ");

        if (cmd.count())
        {
            for (const auto & command : Commands)
            {
                if (command.name == cmd[0])
                {
                    if (globalSettings.commandsAllowed || isUserAdmin(DiscordPtr, message))
                    {
                        channelType = DiscordPtr->getDiscordChannelType(message.channelID);
                        if ((command.ChannelPermission == AllowChannelTypes::Any) || (channelType == command.ChannelPermission))
                        {
                            if (command.opensWallet)
                                MyAccount = &RPCMan.getAccount(DiscordPtr->convertSnowflakeToInt64(message.author.ID));
                            else MyAccount = nullptr;

                            if (isCommandAllowedToBeExecuted(DiscordPtr, message, command, channelType))
                                reinterpret_cast<CommandFunc>(command.func)(DiscordPtr, message, command);
                        }
                    }
                    else
                    {
                        DiscordPtr->sendMessage(message.channelID, "Bot Disabled. :cold_sweat:");
                    }
                }
            }
        }
    }
    catch (const Poco::Exception & exp)
    {
        DiscordPtr->sendMessage(message.channelID, "Poco Error: ---" + std::string(exp.what()) + " :cold_sweat:");
    }
    catch (AppGeneralException & exp)
    {
        DiscordPtr->sendMessage(message.channelID, std::string(exp.what()) + " --- " + exp.getGeneralError() + " :cold_sweat:");
    }
}

void DiscordCommands::Help(ITNS_TIPBOT* DiscordPtr, const SleepyDiscord::Message& message, const Command& me)
{
    std::stringstream ss;
    auto channelType = DiscordPtr->getDiscordChannelType(message.channelID);

    ss << "ITNS Bot Commands:\\n";

    ss << "```";
    for (auto cmd : Commands)
    {
        if (isCommandAllowedToBeExecuted(DiscordPtr, message, cmd, channelType))
        {
            ss << cmd.name << " " << cmd.params;
            if (cmd.ChannelPermission != AllowChannelTypes::Any)
            {
                ss << " -- " << AllowChannelTypeNames[cmd.ChannelPermission];
            }
            if (cmd.adminTools)
            {
                ss << " -- ADMIN ONLY";
            }
            ss << "\\n";
        }
    }
    ss << "```";

    DiscordPtr->sendMessage(message.channelID, ss.str());
}

void DiscordCommands::Balance(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message& message, const struct Command & me)
{
    DiscordPtr->sendMessage(message.channelID, Poco::format("%s#%s: Your Balance is %f ITNS and your Unlocked Balance is %f ITNS", message.author.username, message.author.discriminator, MyAccount->getBalance() / ITNS_OFFSET, MyAccount->getUnlockedBalance() / ITNS_OFFSET));
}

void DiscordCommands::MyAddress(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message& message, const struct Command & me)
{

    DiscordPtr->sendMessage(message.channelID, Poco::format("%s#%s: Your ITNS Address is: %s", message.author.username, message.author.discriminator, Account::getWalletAddress(MyAccount->getDiscordID())));
}

void DiscordCommands::History(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message& message, const struct Command & me)
{
    const auto trxs = RPCMan.getTransfers(MyAccount->getDiscordID());

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
    if (globalSettings.withdrawAllowed)
    {
        Poco::StringTokenizer cmd(message.content, " ");

        if (cmd.count() != 3)
            CommandParseError(DiscordPtr, message, me);
        else
        {
            const auto amount = Poco::NumberParser::parseFloat(cmd[1]);
            const auto& address = cmd[2];
            const auto tx = MyAccount->transferMoneyToAddress(static_cast<std::uint64_t>(amount * ITNS_OFFSET), address);
            DiscordPtr->sendMessage(message.channelID, Poco::format("%s#%s: Withdraw Complete Sent %f ITNS with TX Hash: %s :smiley:", message.author.username, message.author.discriminator, amount, tx.tx_hash));
        }
    }
    else
    {
        DiscordPtr->sendMessage(message.channelID, Poco::format("%s#%s: Withdraws aren't allowed at the moment.", message.author.username, message.author.discriminator));
    }
}

void DiscordCommands::WithdrawAll(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message& message, const struct Command & me)
{
    if (globalSettings.withdrawAllowed)
    {
        Poco::StringTokenizer cmd(message.content, " ");

        if (cmd.count() != 2)
            CommandParseError(DiscordPtr, message, me);
        else
        {
            const auto& address = cmd[1];
            const auto tx = MyAccount->transferAllMoneyToAddress(address);
            DiscordPtr->sendMessage(message.channelID, Poco::format("%s#%s: Withdraw Complete Sent %f ITNS with TX Hash: %s :smiley:", message.author.username, message.author.discriminator, MyAccount->getBalance() / ITNS_OFFSET, tx.tx_hash));
        }
    }
    else
    {
        DiscordPtr->sendMessage(message.channelID, Poco::format("%s#%s: Withdraws aren't allowed at the moment. :cold_sweat:", message.author.username, message.author.discriminator));
    }
}

void DiscordCommands::Give(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message& message, const struct Command & me)
{
    if (globalSettings.giveAllowed)
    {
        Poco::StringTokenizer cmd(message.content, " ");

        if (cmd.count() < 2 || message.mentions.empty())
            CommandParseError(DiscordPtr, message, me);
        else
        {
            const auto amount = Poco::NumberParser::parseFloat(cmd[1]);
            for (const auto& user : message.mentions)
            {
                const auto tx = MyAccount->transferMoneytoAnotherDiscordUser(static_cast<std::uint64_t>(amount * ITNS_OFFSET), DiscordPtr->convertSnowflakeToInt64(user.ID));
                DiscordPtr->sendMessage(message.channelID, Poco::format("%s#%s: Giving %f ITNS to %s with TX Hash: %s :smiley:", message.author.username, message.author.discriminator, amount, user.username, tx.tx_hash));
            }
        }
    }
    else
    {
        DiscordPtr->sendMessage(message.channelID, Poco::format("%s#%s: Give isn't allowed at the moment. :cold_sweat:", message.author.username, message.author.discriminator));
    }
}

void DiscordCommands::GiveAll(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message& message, const struct Command & me)
{
    if (globalSettings.giveAllowed)
    {
        Poco::StringTokenizer cmd(message.content, " ");

        if (cmd.count() != 2 || message.mentions.empty())
            CommandParseError(DiscordPtr, message, me);
        else
        {
            const auto tx = MyAccount->transferAllMoneytoAnotherDiscordUser(DiscordPtr->convertSnowflakeToInt64(message.mentions[0].ID));
            DiscordPtr->sendMessage(message.channelID, Poco::format("%s#%s: Giving %f ITNS to %s with TX Hash: %s :smiley:", message.author.username, message.author.discriminator, static_cast<double>(MyAccount->getBalance() / ITNS_OFFSET), message.mentions[0].username, tx.tx_hash));
        }
    }
    else
    {
        DiscordPtr->sendMessage(message.channelID, Poco::format("%s#%s: Give isn't allowed at the moment. :cold_sweat:", message.author.username, message.author.discriminator));
    }
}

void DiscordCommands::About(ITNS_TIPBOT* DiscordPtr, const SleepyDiscord::Message& message, const Command& me)
{
    DiscordPtr->sendMessage(message.channelID, Poco::format(aboutStr, VERSION_MAJOR, VERSION_MINOR));
}

void DiscordCommands::BlockHeight(ITNS_TIPBOT* DiscordPtr, const SleepyDiscord::Message& message, const Command& me)
{
    DiscordPtr->sendMessage(message.channelID, Poco::format("Your wallet's current block height is: %?i", MyAccount->getBlockHeight()));
}

void DiscordCommands::ToggleWithdrawAllowed(ITNS_TIPBOT* DiscordPtr, const SleepyDiscord::Message& message, const Command& me)
{
    globalSettings.withdrawAllowed = !globalSettings.withdrawAllowed;
    DiscordPtr->sendMessage(message.channelID, Poco::format("Withdraw Enabled: %b", globalSettings.withdrawAllowed));
}

void DiscordCommands::ToggleGiveAllowed(ITNS_TIPBOT* DiscordPtr, const SleepyDiscord::Message& message, const Command& me)
{
    globalSettings.giveAllowed = !globalSettings.giveAllowed;
    DiscordPtr->sendMessage(message.channelID, Poco::format("Give Enabled: %b", globalSettings.giveAllowed));
}

void DiscordCommands::ToggleCommandsAllowed(ITNS_TIPBOT* DiscordPtr, const SleepyDiscord::Message& message, const Command& me)
{
    globalSettings.commandsAllowed = !globalSettings.commandsAllowed;
    DiscordPtr->sendMessage(message.channelID, Poco::format("Bot Enabled: %b", globalSettings.commandsAllowed));
}

bool DiscordCommands::isUserAdmin(ITNS_TIPBOT* DiscordPtr, const SleepyDiscord::Message & message)
{
    auto myid = DiscordPtr->convertSnowflakeToInt64(message.author.ID);
    for (auto adminid : DiscordAdmins)
    {
        if (myid == adminid)
            return true;
    }
    return false;
}

void DiscordCommands::CommandParseError(ITNS_TIPBOT* DiscordPtr, const SleepyDiscord::Message& message, const Command& me)
{
    DiscordPtr->sendMessage(message.channelID, Poco::format("Command Error --- Correct Usage: %s %s :cold_sweat:", me.name, me.params));
}
