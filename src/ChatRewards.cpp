#include "ChatRewards.h"
#include <Poco/Timestamp.h>
#include <Poco/Thread.h>
#include "Util.h"
#include <thread>
#include "RPCManager.h"
#include <random>
#include <algorithm>
#include <fstream>
#include "cereal/cereal.hpp"
#include "cereal/archives/json.hpp"
#include <cereal/types/set.hpp>
#include <cereal/types/stack.hpp>
#include "Language.h"
#include <Poco/StringTokenizer.h>

#define CHATREWARDS_SAVE_FILE "ChatRewards.json"
#define CLASS_RESOLUTION(x) std::bind(&ChatRewards::x, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
ChatRewards::ChatRewards(TIPBOT * DP) : DiscordPtr(DP), lastTimePaymentWasSent(0), channel(0), lastTimeUserDrawn(0), enabled(true)
{
    PLog = &Poco::Logger::get("ChatRewards");

    Commands =
    {
        // User Commands 
        // Command              Function                                      Params                              Wallet  Admin   Allowed Channel
        { "!chatrewards",       CLASS_RESOLUTION(help),                        "",                                 false,  true,   AllowChannelTypes::Private },
        { "!disallowid",        CLASS_RESOLUTION(DisallowID),                  "[id]",                             false,  true,   AllowChannelTypes::Private },
        { "!allowid",           CLASS_RESOLUTION(AllowID),                     "[id]",                             false,  true,   AllowChannelTypes::Private },
        { "!setchannel",        CLASS_RESOLUTION(SetChannel),                  "[id]",                             false,  true,   AllowChannelTypes::Private },
        { "!paymentqueuesize",  CLASS_RESOLUTION(PaymentQueueSize),            "",                                 false,  true,   AllowChannelTypes::Private },
        { "!roundusersize",     CLASS_RESOLUTION(RoundUserSize),               "",                                 false,  true,   AllowChannelTypes::Private },
        { "!togglechatrewards", CLASS_RESOLUTION(ToggleChatRewards),           "",                                 false,  true,   AllowChannelTypes::Private },
    };

    // Create Chat Rewards thread
    std::thread t1(&ChatRewards::ProcessPendingTransfers, this);
    t1.detach();
}

void ChatRewards::run(const UserMessage & message)
{
    const auto & user = DiscordPtr->findUser(message.User.id);
    const Poco::Timestamp   current;
    const std::uint64_t     currentTime = current.epochMicroseconds();
    const auto&             joinTime = user.join_epoch_time;

    // Check if user discord account is old enough.
    if (enabled)
        if ((currentTime - joinTime) >= GlobalConfig.Faucet.min_discord_account)
            if (message.User.id != RPCMan->getBotDiscordID())
                if (message.ChannelPerm != AllowChannelTypes::CLI)
                    if (!isUserDisallowed(message.User.id))
                        if (!channel || channel == message.Channel.id)
                            if (Users.insert(message.User).second)
                            {
                                PLog->information("User %s has been added to the award list.", message.User.username);
                                save();
                            }
}

void ChatRewards::ProcessPendingTransfers()
{
    GlobalConfig.General.Threads++;

    while (!GlobalConfig.General.Shutdown)
    {
        if (enabled)
        {
            const Poco::Timestamp   current;
            const std::uint64_t     currentTime = current.epochMicroseconds();

            if (currentTime > lastTimePaymentWasSent)
            {
                if (!PendingTransfers.empty())
                {
                    Snowflake usr = PendingTransfers.top();
                    PLog->information("Sending payment to user %?i", usr.id);
                    auto & myAccountPtr = RPCManager::getGlobalBotAccount();

                    if (myAccountPtr.getUnlockedBalance())
                    {
                        try
                        {
                            const auto amount = static_cast<std::uint64_t>(myAccountPtr.getUnlockedBalance()*GlobalConfig.Faucet.percentage_allowance);
                            const auto tx = myAccountPtr.transferMoneyToAddress(amount, Account::getWalletAddress(usr.id));

                            DiscordPtr->SendDirectMsg(usr.id, Poco::format("You've been granted %0.8f %s amount for chatting TX Hash %s", amount / GlobalConfig.RPC.coin_offset, GlobalConfig.RPC.coin_abbv, tx.tx_hash));

                            // Maybe send user a DM?
                            PLog->information("Payment success! TX Hash %s", tx.tx_hash);

                            // Success!
                            PendingTransfers.pop();
                        }
                        catch (...)
                        {
                            PLog->information("Payment failed, user requeued. :(");
                            // Fail :(
                        }
                    }
                    else PLog->information("Can't send chat reward because Tipbot is broke");
                }
                lastTimePaymentWasSent = current.epochMicroseconds() + GlobalConfig.ChatRewards.next_payment_time;
                save();
            }

            if (currentTime > lastTimeUserDrawn)
            {
                PLog->information("Randomly choosing user who spoke in the last 30 minutes.");
                std::vector<Snowflake> ToBeShuffled(Users.begin(), Users.end());

                if (!ToBeShuffled.empty())
                {
                    std::shuffle(ToBeShuffled.begin(), ToBeShuffled.end(), std::mt19937(std::random_device()()));
                    PLog->information(Poco::format(GETSTR(DiscordPtr->getUserLang(ToBeShuffled.begin()->id), "CHAT_REWARDS_PAYMENT"), ToBeShuffled.begin()->id));
                    PendingTransfers.push(*ToBeShuffled.begin());
                    Users.clear();
                }

                lastTimeUserDrawn = current.epochMicroseconds() + GlobalConfig.ChatRewards.next_drawing_time;
                save();
            }
        }
        // Sleep for some long amount of time.
        Poco::Thread::sleep(100);
    }

    GlobalConfig.General.Threads--;
}

bool ChatRewards::isUserDisallowed(const DiscordID & id)
{
    for (const auto & usr : NotAllowedIDs)
        if (id == usr) return true;
    return false;
}

void ChatRewards::save()
{
    std::ofstream out(CHATREWARDS_SAVE_FILE, std::ios::trunc);
    if (out.is_open())
    {
        PLog->information("Saving chat rewards data to disk...");
        {
            cereal::JSONOutputArchive ar(out);
            ar(CEREAL_NVP(Users), CEREAL_NVP(PendingTransfers), CEREAL_NVP(lastTimePaymentWasSent), CEREAL_NVP(lastTimeUserDrawn), CEREAL_NVP(channel), CEREAL_NVP(NotAllowedIDs), CEREAL_NVP(enabled));
        }
        out.close();
    }
}

void ChatRewards::load()
{
    std::ifstream in(CHATREWARDS_SAVE_FILE);
    if (in.is_open())
    {
        PLog->information("Loading chat rewards data from disk...");
        {
            cereal::JSONInputArchive ar(in);
            ar(CEREAL_NVP(Users), CEREAL_NVP(PendingTransfers), CEREAL_NVP(lastTimePaymentWasSent), CEREAL_NVP(lastTimeUserDrawn), CEREAL_NVP(channel), CEREAL_NVP(NotAllowedIDs), CEREAL_NVP(enabled));
        }
        in.close();
    }
}

void ChatRewards::setAccount(Account *)
{
}

iterator ChatRewards::begin()
{
    return Commands.begin();
}

const_iterator ChatRewards::begin() const
{
    return Commands.begin();
}

const_iterator ChatRewards::cbegin() const
{
    return Commands.cbegin();
}

iterator ChatRewards::end()
{
    return Commands.end();
}

const_iterator ChatRewards::end() const
{
    return Commands.end();

}

const_iterator ChatRewards::cend() const
{
    return Commands.cend();
}

void ChatRewards::help(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me)
{
    const auto helpStr = TIPBOT::generateHelpText(GETSTR(DiscordPtr->getUserLang(message.User.id), "TIP_HELP_COMMAND"), Commands, message);
    DiscordPtr->SendMsg(message, helpStr);
}

void ChatRewards::DisallowID(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me)
{
    Poco::StringTokenizer cmd(message.Message, " ");

    if (cmd.count() != 2)
        DiscordPtr->CommandParseError(message, me);
    else
    {
        const auto discordId = Poco::NumberParser::parseUnsigned64(cmd[1]);
        NotAllowedIDs.insert(discordId);
        save();
        DiscordPtr->SendMsg(message, "User added to the disallowed list!");
    }
}

void ChatRewards::AllowID(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me)
{
    Poco::StringTokenizer cmd(message.Message, " ");

    if (cmd.count() != 2)
        DiscordPtr->CommandParseError(message, me);
    else
    {
        const auto discordId = Poco::NumberParser::parseUnsigned64(cmd[1]);
        NotAllowedIDs.erase(discordId);
        save();
        DiscordPtr->SendMsg(message, "User removed to the disallowed list!");
    }
}

void ChatRewards::SetChannel(TIPBOT * DiscordPtr, const UserMessage & message, const Command & me)
{
    Poco::StringTokenizer cmd(message.Message, " ");

    if (cmd.count() != 2)
        DiscordPtr->CommandParseError(message, me);
    else
    {
        const auto discordChannel = Poco::NumberParser::parseUnsigned64(cmd[1]);
        channel = discordChannel;
        save();
        DiscordPtr->SendMsg(message, "Channel Set!");
    }
}

void ChatRewards::PaymentQueueSize(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me)
{
    DiscordPtr->SendMsg(message, Poco::format("Payment Queue Size %?i", PendingTransfers.size()));
}

void ChatRewards::RoundUserSize(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me)
{
    DiscordPtr->SendMsg(message, Poco::format("This rounds user count %?i", Users.size()));
}

void ChatRewards::ToggleChatRewards(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me)
{
    enabled = !enabled;
    save();
    PLog->information("Chat Rewards Status: %b", enabled);
    DiscordPtr->SendMsg(message, Poco::format("Chat Rewards Enabled: %b", enabled));
}