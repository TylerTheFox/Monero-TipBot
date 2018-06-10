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
#include "Language.h"

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
        { "!listlanguage",    CLASS_RESOLUTION(ListLanguages),               "",                                 false,  false,  AllowChannelTypes::Any },
        { "!selectlanguage",  CLASS_RESOLUTION(SelectLanguage),              "",                                 false,  false,  AllowChannelTypes::Any },
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
        { "!whois",           CLASS_RESOLUTION(WhoIs),                      "[DiscordID]",                       false,  true,   AllowChannelTypes::Private },
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
    const auto helpStr = TIPBOT::generateHelpText(GETSTR(DiscordPtr->getUserLang(message.author.ID), "TIP_HELP_COMMAND"), Commands, channelType, message);
    DiscordPtr->sendMessage(message.channelID, helpStr);
}

void Tip::Balance(TIPBOT * DiscordPtr, const SleepyDiscord::Message& message, const struct Command & me)
{
    DiscordPtr->sendMessage(message.channelID, Poco::format(GETSTR(DiscordPtr->getUserLang(message.author.ID), "TIP_BALANCE"), message.author.username, message.author.discriminator, MyAccount->getBalance() / GlobalConfig.RPC.coin_offset, GlobalConfig.RPC.coin_abbv, MyAccount->getUnlockedBalance() / GlobalConfig.RPC.coin_offset, GlobalConfig.RPC.coin_abbv));
}

void Tip::MyAddress(TIPBOT * DiscordPtr, const SleepyDiscord::Message& message, const struct Command & me)
{
    DiscordPtr->sendMessage(message.channelID, Poco::format(GETSTR(DiscordPtr->getUserLang(message.author.ID), "TIP_ADDRESS"), message.author.username, message.author.discriminator, GlobalConfig.RPC.coin_abbv, Account::getWalletAddress(TIPBOT::convertSnowflakeToInt64(message.author.ID))));
}

void Tip::History(TIPBOT * DiscordPtr, const SleepyDiscord::Message& message, const struct Command & me)
{
    const auto trxs = MyAccount->getTransactions();

    std::stringstream ss;

    const auto addtoss = [&ss, DiscordPtr, &message](const std::multiset<struct TransferItem, TransferItemCmp> & sset)
    {
        auto i = 0;
        ss << GETSTR(DiscordPtr->getUserLang(message.author.ID), "TIP_HISTORY_HEADER");

        for (auto tx : sset)
        {
            if (i == 5) break;
            ss << tx.amount / GlobalConfig.RPC.coin_offset << " | " << DiscordPtr->findUser(tx.payment_id).username << " | " << tx.block_height << " | " << tx.tx_hash << "\\n";
            i++;
        }
        ss << "```";
    };

    ss << GETSTR(DiscordPtr->getUserLang(message.author.ID), "TIP_HISTORY_INC_HEADER");
    addtoss(trxs.tx_in);

    ss << GETSTR(DiscordPtr->getUserLang(message.author.ID), "TIP_HISTORY_OUT_HEADER");
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
            DiscordPtr->sendMessage(message.channelID, Poco::format(GETSTR(DiscordPtr->getUserLang(message.author.ID), "TIP_WITHDRAW_SUCCESS"), message.author.username, message.author.discriminator, amount, GlobalConfig.RPC.coin_abbv, tx.tx_hash));
        }
    }
    else
    {
        DiscordPtr->sendMessage(message.channelID, Poco::format(GETSTR(DiscordPtr->getUserLang(message.author.ID), "TIP_WITHDRAW_SUSPENDED"), message.author.username, message.author.discriminator));
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
            DiscordPtr->sendMessage(message.channelID, Poco::format(GETSTR(DiscordPtr->getUserLang(message.author.ID), "TIP_WITHDRAW_SUCCESS"), message.author.username, message.author.discriminator, MyAccount->getUnlockedBalance() / GlobalConfig.RPC.coin_offset, GlobalConfig.RPC.coin_abbv, tx.tx_hash));
        }
    }
    else
    {
        DiscordPtr->sendMessage(message.channelID, Poco::format(GETSTR(DiscordPtr->getUserLang(message.author.ID), "TIP_WITHDRAW_SUSPENDED"), message.author.username, message.author.discriminator));
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
                DiscordPtr->sendMessage(message.channelID, Poco::format(GETSTR(DiscordPtr->getUserLang(message.author.ID), "TIP_GIVE_SUCESS"), message.author.username, message.author.discriminator, amount, GlobalConfig.RPC.coin_abbv, user.username, tx.tx_hash));
            }
        }
    }
    else
    {
        DiscordPtr->sendMessage(message.channelID, Poco::format(GETSTR(DiscordPtr->getUserLang(message.author.ID), "TIP_GIVE_SUSPENDED"), message.author.username, message.author.discriminator));
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
            DiscordPtr->sendMessage(message.channelID, Poco::format(GETSTR(DiscordPtr->getUserLang(message.author.ID), "TIP_GIVE_SUCESS"), message.author.username, message.author.discriminator, static_cast<double>(MyAccount->getUnlockedBalance() / GlobalConfig.RPC.coin_offset), GlobalConfig.RPC.coin_abbv, message.mentions[0].username, tx.tx_hash));
        }
    }
    else
    {
        DiscordPtr->sendMessage(message.channelID, Poco::format(GETSTR(DiscordPtr->getUserLang(message.author.ID), "TIP_GIVE_SUSPENDED"), message.author.username, message.author.discriminator));
    }
}

void Tip::About(TIPBOT* DiscordPtr, const SleepyDiscord::Message& message, const Command& me)
{
    DiscordPtr->sendMessage(message.channelID, Poco::format(aboutStr, VERSION_MAJOR, VERSION_MINOR, GlobalConfig.About.major, GlobalConfig.About.minor));
}

void Tip::BlockHeight(TIPBOT* DiscordPtr, const SleepyDiscord::Message& message, const Command& me)
{
    DiscordPtr->sendMessage(message.channelID, Poco::format(GETSTR(DiscordPtr->getUserLang(message.author.ID), "TIP_BLOCK_HEIGHT"), MyAccount->getBlockHeight()));
}

void Tip::RestartWallet(TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const Command & me)
{
    RPCMan->restartWallet(MyAccount->getDiscordID());
    DiscordPtr->sendMessage(message.channelID, GETSTR(DiscordPtr->getUserLang(message.author.ID), "TIP_WALLET_RESTART_SUCCESS"));
}

void Tip::ToggleWithdraw(TIPBOT* DiscordPtr, const SleepyDiscord::Message& message, const Command& me)
{
    globalSettings.withdrawAllowed = !globalSettings.withdrawAllowed;
    DiscordPtr->sendMessage(message.channelID, Poco::format(GETSTR(DiscordPtr->getUserLang(message.author.ID), "TIP_WITHDRAW_TOGGLE"), globalSettings.withdrawAllowed));
}

void Tip::ToggleGive(TIPBOT* DiscordPtr, const SleepyDiscord::Message& message, const Command& me)
{
    globalSettings.giveAllowed = !globalSettings.giveAllowed;
    DiscordPtr->sendMessage(message.channelID, Poco::format(GETSTR(DiscordPtr->getUserLang(message.author.ID), "TIP_GIVE_TOGGLE"), globalSettings.giveAllowed));
}

void Tip::RescanAllWallets(TIPBOT* DiscordPtr, const SleepyDiscord::Message& message, const Command& me)
{
    RPCMan->rescanAll();
    DiscordPtr->sendMessage(message.channelID, GETSTR(DiscordPtr->getUserLang(message.author.ID), "TIP_RESCAN_SUCCESS"));
}

void Tip::TotalBalance(TIPBOT* DiscordPtr, const SleepyDiscord::Message& message, const Command& me)
{
    DiscordPtr->sendMessage(message.channelID, Poco::format(GETSTR(DiscordPtr->getUserLang(message.author.ID), "TIP_TOTAL_BALANCE"), RPCMan->getTotalBalance() / GlobalConfig.RPC.coin_offset, GlobalConfig.RPC.coin_abbv, RPCMan->getTotalUnlockedBalance() / GlobalConfig.RPC.coin_offset, GlobalConfig.RPC.coin_abbv));
}

void Tip::SaveWallets(TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const Command & me)
{
    RPCMan->saveallWallets();
    DiscordPtr->sendMessage(message.channelID, GETSTR(DiscordPtr->getUserLang(message.author.ID), "TIP_WALLET_SAVE_SUCCESS"));
}

void Tip::RestartFaucetWallet(TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const Command & me)
{
    RPCMan->restartWallet(RPCMan->getBotDiscordID());
    DiscordPtr->sendMessage(message.channelID, GETSTR(DiscordPtr->getUserLang(message.author.ID), "TIP_FAUCET_RESTART_SUCCESS"));
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
    DiscordPtr->sendMessage(message.channelID, GETSTR(DiscordPtr->getUserLang(message.author.ID), "TIP_RESTART_SUCCESSS"));
    GlobalConfig.General.Shutdown = true;
    DiscordPtr->quit();
}

void Tip::Shutdown(TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me)
{
    DiscordPtr->sendMessage(message.channelID, GETSTR(DiscordPtr->getUserLang(message.author.ID), "TIP_SHUTDOWN_SUCCESSS"));
    GlobalConfig.General.Shutdown = true;
    GlobalConfig.General.Quitting = true;
    DiscordPtr->quit();
}

void Tip::RPCStatus(TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me)
{
    DiscordPtr->sendMessage(message.channelID, RPCMan->status());
}

void Tip::WhoIs(TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const Command & me)
{
    Poco::StringTokenizer cmd(message.content, " ");

    if (cmd.count() != 2)
        DiscordPtr->CommandParseError(message, me);
    else
    {
        const auto discordId = Poco::NumberParser::parseUnsigned64(cmd[1]);
        auto user = DiscordPtr->findUser(discordId);
        if (user.id == discordId)
            DiscordPtr->sendMessage(message.channelID, Poco::format(GETSTR(DiscordPtr->getUserLang(message.author.ID), "TIP_WHOIS_USER"), discordId, user.username));
        else
            DiscordPtr->sendMessage(message.channelID, GETSTR(DiscordPtr->getUserLang(message.author.ID), "TIP_WHOIS_USER_NOT_FOUND"));
    }
}

void Tip::ListLanguages(TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me)
{
    std::vector<std::string> vect;
    GlobalLanguage.getLanguages(vect);

    std::stringstream ss;

    ss << GETSTR(DiscordPtr->getUserLang(message.author.ID), "TIP_LIST_LANGUAGE");
    ss << "```";
    for (int i = 0; i < vect.size(); i++)
        ss << Poco::format(GETSTR(DiscordPtr->getUserLang(message.author.ID), "TIP_LIST_LANGUAGE_FORMAT"), i, vect[i]);
    ss << "```";

    DiscordPtr->sendMessage(message.channelID, ss.str());
}

void Tip::SelectLanguage(TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me)
{
    Poco::StringTokenizer cmd(message.content, " ");

    if (cmd.count() != 2)
        DiscordPtr->CommandParseError(message, me);
    else
    {
        const auto langid = Poco::NumberParser::parseUnsigned(cmd[1]);
        auto & user = DiscordPtr->findUser(DiscordPtr->convertSnowflakeToInt64(message.author.ID));

        if (langid < GlobalLanguage.size())
        {
            user.language = langid;
            DiscordPtr->sendMessage(message.channelID, GETSTR(DiscordPtr->getUserLang(message.author.ID), "TIP_LANGUAGE_SELECT_SUCCESS"));
        }
        else DiscordPtr->sendMessage(message.channelID, GETSTR(DiscordPtr->getUserLang(message.author.ID), "TIP_LANGUAGE_SELECT_FAIL"));
    }
}
