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
#include "Tipbot.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/Timestamp.h"
#include "Poco/Timespan.h"
#include "RPCManager.h"
#include <utility>
#include <map>
#include "Config.h"
#include "RPCException.h"
#include "Language.h"
#include <fstream>
#include "cereal/cereal.hpp"
#include "cereal/archives/json.hpp"

#define CLASS_RESOLUTION(x) std::bind(&Faucet::x, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
Faucet::Faucet() : enabled(true)
{
    Commands =
    {
        // User Commands 
        // Command            Function                                      Params                              Wallet  Admin   Allowed Channel
        { "!faucet",          CLASS_RESOLUTION(help),                       "",                                 false,  false,  AllowChannelTypes::Any },
        { "!take",            CLASS_RESOLUTION(take),                       "",                                 false,  false,  AllowChannelTypes::Public },
        { "!status",          CLASS_RESOLUTION(status),                     "",                                 false,  true,   AllowChannelTypes::Private },
        { "!togglefaucet",    CLASS_RESOLUTION(ToggleFaucet),               "",                                 false,  true,   AllowChannelTypes::Private },
    };
    PLog = &Poco::Logger::get("Faucet");
}

void Faucet::save()
{
    std::ofstream out(FAUCET_SAVE_FILE, std::ios::trunc);
    if (out.is_open())
    {
        PLog->information("Saving faucet data to disk...");
        {
            cereal::JSONOutputArchive ar(out);
            ar(CEREAL_NVP(enabled));
        }
        out.close();
    }
}

void Faucet::load()
{
    std::ifstream in(FAUCET_SAVE_FILE, std::ios::trunc);
    if (in.is_open())
    {
        PLog->information("Loading faucet data from disk...");
        {
            cereal::JSONInputArchive ar(in);
            ar(CEREAL_NVP(enabled));
        }
        in.close();
    }
}

void Faucet::setAccount(Account*)
{
    // Do nothing, we construct this parameter since its pure virtual and we dont need it in this class.
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

void Faucet::help(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me) const
{
    const auto helpStr = TIPBOT::generateHelpText(GETSTR(DiscordPtr->getUserLang(message.User.id), "FAUCET_HELP"), Commands, message);
    DiscordPtr->SendMsg(message, helpStr);
}

void Faucet::take(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me)
{
    std::stringstream ss;
    if (enabled)
    {
        auto & myAccountPtr = RPCManager::getGlobalBotAccount();

        myAccountPtr.resyncAccount();

        const auto & user = DiscordPtr->findUser(message.User.id);
        const Poco::Timestamp   current;
        const std::uint64_t     currentTime = current.epochMicroseconds();
        const auto&             joinTime = user.join_epoch_time;
        const auto&             faucetTime = user.faucet_epoch_time;

        //  Security check: Is this the real faucet account?
        if (Account::getWalletAddress(RPCMan->getBotDiscordID()) != myAccountPtr.getMyAddress())
            throw RPCGeneralError("-1", "Faucet Account Error.");

        if ((currentTime - joinTime) >= GlobalConfig.Faucet.min_discord_account)
        {
            if (currentTime > faucetTime)
            {
                if (myAccountPtr.getUnlockedBalance())
                {
                    const auto amount = static_cast<std::uint64_t>(myAccountPtr.getUnlockedBalance()*GlobalConfig.Faucet.percentage_allowance);
                    const auto tx = myAccountPtr.transferMoneyToAddress(amount, Account::getWalletAddress(message.User.id));
                    user.faucet_epoch_time = current.epochMicroseconds() + GlobalConfig.Faucet.timeout;
                    user.total_faucet_itns_sent += amount;
                    PLog->information("User %s was issued %0.8f %s with TX Hash %s", message.User.id_str, amount / GlobalConfig.RPC.coin_offset, GlobalConfig.RPC.coin_abbv, tx.tx_hash);
                    ss << Poco::format(GETSTR(DiscordPtr->getUserLang(message.User.id), "FAUCET_TAKE_SUCCESS"), message.User.username, message.User.discriminator, amount / GlobalConfig.RPC.coin_offset, GlobalConfig.RPC.coin_abbv, tx.tx_hash);
                    DiscordPtr->saveUserList();
                }
                else if (myAccountPtr.getBalance())
                    ss << GETSTR(DiscordPtr->getUserLang(message.User.id), "FAUCET_TAKE_PENDING_TRANSACTIONS");
                else ss << GETSTR(DiscordPtr->getUserLang(message.User.id), "FAUCET_TAKE_IS_BROKE");
            }
            else ss << Poco::format(GETSTR(DiscordPtr->getUserLang(message.User.id), "FAUCET_TAKE_TOO_SOON"), GlobalConfig.Faucet.timeout / MICROSECOND_HOUR, static_cast<double>(faucetTime - currentTime) / MICROSECOND_HOUR);
        }
        else ss << GETSTR(DiscordPtr->getUserLang(message.User.id), "FAUCET_TAKE_ACCOUNT_NOT_MATURE");
    }
    else ss << GETSTR(DiscordPtr->getUserLang(message.User.id), "FAUCET_TAKE_DISABLED");
    DiscordPtr->SendMsg(message, ss.str());
}

void Faucet::status(TIPBOT* DiscordPtr, const UserMessage& message, const Command& me) const
{
    auto & myAccountPtr = RPCManager::getGlobalBotAccount();
    const auto & user = DiscordPtr->findUser(message.User.id);

    std::stringstream ss;
    ss.precision(8);
    myAccountPtr.resyncAccount();

    auto txs = myAccountPtr.getTransactions();
    std::uint64_t recieved = 0;
    std::uint64_t sent = 0;

    std::map<DiscordID, std::uint64_t> topDonorList;
    for (auto tx : txs.tx_in)
    {
        if (tx.payment_id > 0)
            topDonorList[tx.payment_id] += tx.amount;
        recieved += tx.amount;
    }

    for (auto tx : txs.tx_out)
    {
        sent += tx.amount;
    }

    auto TopDonor = std::max_element(topDonorList.begin(), topDonorList.end(),
        [](const std::pair<DiscordID, std::uint64_t>& p1, const std::pair<DiscordID, std::uint64_t>& p2) {
        return p1.second < p2.second; });


    auto TopTaker = DiscordPtr->findTopTaker();

    ss << "```";
    ss << "Your name is: " << user.username << ".\\n";
    ss << "Your ID is: " << user.id << ".\\n";
    ss << "Bot ID is: " << myAccountPtr.getDiscordID() << ".\\n";
    ss << "Bot current balance is: " << myAccountPtr.getBalance() / GlobalConfig.RPC.coin_offset << ".\\n";
    ss << "Bot current unlocked balance is: " << myAccountPtr.getUnlockedBalance() / GlobalConfig.RPC.coin_offset << ".\\n";
    ss << "Bot current address is: " << myAccountPtr.getMyAddress() << ".\\n";
    ss << "Bot timeout is: " << GlobalConfig.Faucet.timeout / MICROSECOND_HOUR << " hours\\n";
    ss << "Minimum Discord Account: " << GlobalConfig.Faucet.min_discord_account / MICROSECOND_DAY << " days.\\n";
    ss << "Current Award: " << (myAccountPtr.getUnlockedBalance()*GlobalConfig.Faucet.percentage_allowance) / GlobalConfig.RPC.coin_offset << ".\\n";
    ss << "Current payout percentage: " << GlobalConfig.Faucet.percentage_allowance * 100 << "%.\\n";
    ss << "Current Amount Awarded: " << sent / GlobalConfig.RPC.coin_offset << ".\\n";
    ss << "Current Donated From Users: " << recieved / GlobalConfig.RPC.coin_offset << ".\\n";
    if (!topDonorList.empty())
    {
        const auto & TopDonorUser = DiscordPtr->findUser(TopDonor->first);
        ss << "Current Top Donor: " << TopDonorUser.username << " (" << TopDonorUser.id << ").\\n";
        ss << "Current Top Donor Amount: " << (TopDonor->second / GlobalConfig.RPC.coin_offset) << ".\\n";
    }
    ss << "Current Top Taker: " << TopTaker.me.username << " (" << TopTaker.me.id << ").\\n";
    ss << "Current Top Taker Amount: " << (TopTaker.amount / GlobalConfig.RPC.coin_offset) << ".\\n";
    ss << "Faucet Enabled: " << enabled << ".\\n";
    ss << "```";

    DiscordPtr->SendMsg(message, ss.str());
}

void Faucet::ToggleFaucet(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me)
{
    enabled = !enabled;
    DiscordPtr->AppSave();
    PLog->information("Faucet Status: %b", enabled);
    DiscordPtr->SendMsg(message, Poco::format("Faucet Enabled: %b", enabled));
}