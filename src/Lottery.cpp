#include "Lottery.h"
#include "Discord.h"
#include "RPCManager.h"
#include <functional>
#include <fstream>
#include "cereal/archives/json.hpp"
#include "cereal/types/list.hpp"
#include "Poco/StringTokenizer.h"
#include "Poco/Thread.h"
#include "Config.h"
#include "RPCException.h"
#include "Language.h"

#define CLASS_RESOLUTION(x) std::bind(&Lottery::x, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
Lottery::Lottery(TIPBOT * DP) : DiscordPtr(DP), lotterySuspended(false), prevWinner(0)
{
    Commands =
    {
        // User Commands 
        // Command                      Function                                      Params            Wallet  Admin   Allowed Channel
        { "!lottery",                   CLASS_RESOLUTION(LotteryHelp),                "",               false,  false,  AllowChannelTypes::Any        },
        { "!jackpot",                   CLASS_RESOLUTION(Jackpot),                    "",               false,  false,  AllowChannelTypes::Any        },
        { "!gameinfo",                  CLASS_RESOLUTION(gameInfo),                   "",               false,  false,  AllowChannelTypes::Any        },
        { "!mytickets",                 CLASS_RESOLUTION(MyTickets),                  "",               false,  false,  AllowChannelTypes::Any        },
        { "!buytickets",                CLASS_RESOLUTION(BuyTicket),                  "[amount]",       true,   false,  AllowChannelTypes::Any        },
        { "!waslotterywon",             CLASS_RESOLUTION(LotteryWon),                 "",               false,  false,  AllowChannelTypes::Any        },

        { "!togglelotterysuspend",      CLASS_RESOLUTION(ToggleLotterySuspend),       "",               false,  true,   AllowChannelTypes::Private    },
        { "!lastwinner",                CLASS_RESOLUTION(lastWinner),                 "",               false,  true,   AllowChannelTypes::Private    },
    };
    LotteryAccount = RPCManager::manuallyCreateRPC(LOTTERY_USER, GlobalConfig.RPCManager.starting_port_number - 1);
    PLog = &Poco::Logger::get("Lottery");
}

Lottery::~Lottery()
{
    try 
    {
        LotteryAccount->MyRPC.store();
        LotteryAccount->MyRPC.stopWallet();
    }
    catch (...)
    {

    }
}

void Lottery::save()
{
    std::ofstream out(LOTTERY_SAVE_FILE, std::ios::trunc);
    if (out.is_open())
    {
        PLog->information("Saving lottery data to disk...");
        {
            cereal::JSONOutputArchive ar(out);
            ar(CEREAL_NVP(lastWinningTopBlock), CEREAL_NVP(prevWinner));
        }
        out.close();
    }
}

void Lottery::load()
{
    std::ifstream in(LOTTERY_SAVE_FILE);
    if (in.is_open())
    {
        PLog->information("Loading lottery data from the disk...");
        {
            cereal::JSONInputArchive ar(in);
            ar(CEREAL_NVP(lastWinningTopBlock));

            if (GlobalConfig.About.major > 2 || GlobalConfig.About.major >= 2 && GlobalConfig.About.minor > 1)
            {
                ar(CEREAL_NVP(prevWinner));
            }
        }
        in.close();
    }

    // Create lottery thread
    std::thread t1(&Lottery::run, this);
    t1.detach();
}

void Lottery::setAccount(Account* acc)
{
    currentUsrAccount = acc;
}

iterator Lottery::begin()
{
    return Commands.begin();
}

const_iterator Lottery::begin() const
{
    return Commands.begin();
}

const_iterator Lottery::cbegin() const
{
    return Commands.cbegin();
}

iterator Lottery::end()
{
    return Commands.end();
}

const_iterator Lottery::end() const
{
    return Commands.end();
}

const_iterator Lottery::cend() const
{
    return Commands.cend();
}

void Lottery::run()
{
    bool rewardGivenout = false;
    bool sweepComplete = false;
    bool noWinner = false;

    GlobalConfig.General.Threads++;

    while (!GlobalConfig.General.Shutdown)
    {
        if (!lotterySuspended)
        {
            Poco::DateTime curr;
            if (!noWinner && !rewardGivenout && curr.dayOfWeek() == GlobalConfig.Lottery.day && curr.hour() == GlobalConfig.Lottery.pick)
            {
                PLog->information("Choosing Winners");
                try
                {
                    LotteryAccount->MyAccount.resyncAccount();

                    // Calcualte jackpot.
                    std::vector<DiscordID> enteries;
                    auto txs = LotteryAccount->MyRPC.getTransfers();
                    if (!txs.tx_in.empty())
                    {
                        std::uint64_t bal = 0;
                        unsigned int tickets;

                        // Add tickets to entry list
                        for (auto tx : txs.tx_in)
                        {
                            if (tx.block_height > lastWinningTopBlock)
                            {
                                tickets = (tx.amount / GlobalConfig.RPC.coin_offset) / GlobalConfig.Lottery.ticket_cost;
                                for (int i = 0; i < tickets; i++)
                                    enteries.emplace_back(tx.payment_id);
                                bal += tx.amount;
                            }
                        }

                        if (!enteries.empty())
                        {
                            // Add 20% empty tickets.
                            const auto amountOfBlankTickets = enteries.size() * GlobalConfig.Lottery.no_winner_chance;
                            for (auto i = 0; i < amountOfBlankTickets; i++)
                                enteries.emplace_back(0);

                            // Randomly shuffle list.
                            std::shuffle(enteries.begin(), enteries.end(), std::mt19937(std::random_device()()));

                            DiscordID winner = *enteries.begin();

                            if (winner)
                            {
                                PLog->information(GETSTR(DiscordPtr->getUserLang(winner), "LOTTERY_WINNER"), winner);
                                lastWinningTopBlock = txs.tx_in.begin()->block_height;
                                const std::uint64_t reward = bal - (bal * GlobalConfig.Lottery.donation_percent);
                                auto WinnerAccount = RPCMan->getAccount(winner);
                                
                                // TODO FIX
                                DiscordPtr->sendMessage(DiscordPtr->getDiscordDMChannel(winner), Poco::format("You've won %0.8f %s from the lottery! :money_with_wings:", reward / GlobalConfig.RPC.coin_offset, GlobalConfig.RPC.coin_abbv));
                                LotteryAccount->MyAccount.transferMoneyToAddress(reward, WinnerAccount.getMyAddress());
                                prevWinner = winner;
                            }
                            else
                            {
                                PLog->information("No Winner!");
                                prevWinner = 0;
                                noWinner = true;
                            }
                            DiscordPtr->AppSave();
                            rewardGivenout = true;
                        } else PLog->information("No Active Tickets!");
                    }
                    else PLog->error("Error transaction list is empty!");
                }
                catch (AppGeneralException & exp)
                {
                    PLog->error("There was an error while in the lottery drawing. Lottery is suspended! Error: %s", exp.getGeneralError());
                    lotterySuspended = true;
                }
                catch (...)
                {
                    PLog->error("There was an unknown error while in the lottery drawing. Lottery is suspended!");
                    lotterySuspended = true;
                }
            }
            else if (!sweepComplete && curr.dayOfWeek() == GlobalConfig.Lottery.day && curr.hour() == GlobalConfig.Lottery.faucet)
            {
                try
                {
                    LotteryAccount->MyAccount.resyncAccount();

                    // Donate Remaining to faucet.
                    if (!noWinner)
                    {
                        PLog->information("Donating remaining balance to the faucet!");
                        LotteryAccount->MyAccount.transferAllMoneyToAddress(RPCManager::getGlobalBotAccount().getMyAddress());
                    }
                    PLog->information("Sweep Complete!");
                    noWinner = false;
                    sweepComplete = true;
                } catch (...)
                {
                    // Don't care, try again in 30 seconds (29 + the sleep at the end of the loop).
                    Poco::Thread::sleep(29000);
                }
            }
            else
            {
                if ((rewardGivenout && sweepComplete) || (rewardGivenout && noWinner && curr.hour() < GlobalConfig.Lottery.close))
                {
                    PLog->information("Lottery complete! Resetting local data.");
                    sweepComplete = false;
                    rewardGivenout = false;
                }
            }
        }
        Poco::Thread::sleep(1000);
    }

    GlobalConfig.General.Threads--;
}

void Lottery::gameInfo(TIPBOT* DiscordPtr, const UserMessage& message, const Command& me) const
{
    DiscordPtr->SendMsg(message, Poco::format(GETSTR(DiscordPtr->getUserLang(message.User.id), "LOTTERY_GAME_INFO"), GlobalConfig.Lottery.ticket_cost, GlobalConfig.RPC.coin_abbv, GlobalConfig.Lottery.donation_percent * 100, GlobalConfig.Lottery.no_winner_chance * 100));
}

void Lottery::LotteryHelp(TIPBOT* DiscordPtr, const UserMessage& message, const Command& me) const
{
    const auto channelType = DiscordPtr->getDiscordChannelType(message.Channel.id_str);
    const auto helpStr = TIPBOT::generateHelpText(GETSTR(DiscordPtr->getUserLang(message.User.id), "LOTTERY_HELP"), Commands, message);
    DiscordPtr->SendMsg(message, helpStr);
}

void Lottery::Jackpot(TIPBOT* DiscordPtr, const UserMessage& message, const Command& me) const
{
    // Calcualte jackpot.
    std::uint64_t bal = 0;
    auto txs = LotteryAccount->MyRPC.getTransfers();
    for (auto tx : txs.tx_in)
    {
        if (tx.block_height > lastWinningTopBlock)
        {
            bal += tx.amount;
        }
    }
    DiscordPtr->SendMsg(message, Poco::format(GETSTR(DiscordPtr->getUserLang(message.User.id), "LOTTERY_JACKPOT"), bal / GlobalConfig.RPC.coin_offset));
}

void Lottery::BuyTicket(TIPBOT* DiscordPtr, const UserMessage& message, const Command& me) const
{
    Poco::DateTime curr;
    if (curr.dayOfWeek() != GlobalConfig.Lottery.day || (curr.dayOfWeek() == GlobalConfig.Lottery.day && curr.hour() < GlobalConfig.Lottery.close))
    {
        if (!lotterySuspended)
        {
            Poco::StringTokenizer cmd(message.Message, " ");

            if (cmd.count() != 2)
                DiscordPtr->CommandParseError(message, me);
            else
            {
                LotteryAccount->MyAccount.resyncAccount();
                const auto tickets = Poco::NumberParser::parseUnsigned(cmd[1]);
                const auto tx = currentUsrAccount->transferMoneyToAddress((tickets * GlobalConfig.Lottery.ticket_cost) * GlobalConfig.RPC.coin_offset, LotteryAccount->MyAccount.getMyAddress());
                DiscordPtr->SendMsg(message, Poco::format(GETSTR(DiscordPtr->getUserLang(message.User.id), "LOTTERY_BUY_TICKET_SUCCESS"), message.User.username, message.User.discriminator, tickets, tickets * GlobalConfig.Lottery.ticket_cost, GlobalConfig.RPC.coin_abbv, tx.tx_hash));
            }
        }
        else DiscordPtr->SendMsg(message, GETSTR(DiscordPtr->getUserLang(message.User.id), "LOTTERY_SUSPENDED"));
    }
    else DiscordPtr->SendMsg(message, GETSTR(DiscordPtr->getUserLang(message.User.id), "LOTTERY_CLOSED"));
}

void Lottery::MyTickets(TIPBOT* DiscordPtr, const UserMessage& message, const Command& me) const
{
    LotteryAccount->MyAccount.resyncAccount();

    // Calcualte jackpot.
    std::uint64_t bal = 0;
    auto txs = LotteryAccount->MyRPC.getTransfers();
    for (auto tx : txs.tx_in)
    {
        if (tx.block_height > lastWinningTopBlock && tx.payment_id == message.User.id)
        {
            bal += tx.amount;
        }
    }
    DiscordPtr->SendMsg(message, Poco::format(GETSTR(DiscordPtr->getUserLang(message.User.id), "LOTTERY_TICKETS"), static_cast<uint64_t>((bal / GlobalConfig.RPC.coin_offset) / GlobalConfig.Lottery.ticket_cost)));
}

void Lottery::LotteryWon(TIPBOT * DiscordPtr, const UserMessage& message, const Command & me) const
{
    if (prevWinner)
        DiscordPtr->SendMsg(message, GETSTR(DiscordPtr->getUserLang(message.User.id), "LOTTERY_WON_WITH_WINNER"));
    else 
        DiscordPtr->SendMsg(message, GETSTR(DiscordPtr->getUserLang(message.User.id), "LOTTERY_WON_WITHOUT_WINNER"));
}

void Lottery::lastWinner(TIPBOT * DiscordPtr, const UserMessage& message, const Command & me) const
{
    DiscordPtr->SendMsg(message, Poco::format(GETSTR(DiscordPtr->getUserLang(message.User.id), "LOTTERY_LAST_WINNER"), prevWinner));
}

void Lottery::ToggleLotterySuspend(TIPBOT* DiscordPtr, const UserMessage& message, const Command& me)
{
    lotterySuspended = !lotterySuspended;    
    PLog->information("Lottery Status: %b", lotterySuspended);
    DiscordPtr->AppSave();
    DiscordPtr->SendMsg(message, Poco::format(GETSTR(DiscordPtr->getUserLang(message.User.id), "LOTTERY_SUSPEND_TOGGLE"), lotterySuspended));
}
