/*
 *
 * Lottery opens at MIDNIGHT Saturdays
 * Lottery closes at 6 PM PDT every Friday
 * Winners announced 9 PM PDT every Friday
 * 5% of the winnings goes to the Faucet.
 *  
 */
#pragma once
#include "Discord.h"
#include "Account.h"
#include "AppBaseClass.h"
#include "RPCManager.h"

#define    LOTTERY_USER            "LOTTERY" // Wallet
#define    LOTTERY_SAVE_FILE       "LOTTERY.JSON"
#define    TICKET_COST             100 // ITNS
#define    FACUET_DONATION_PERCENT 0.20
#define    NO_WINNER_CHANCE        0.20
#define    LOTTERY_DAY             Poco::DateTime::FRIDAY
#define    LOTTERY_CLOSE           18
#define    LOTTERY_PICK            21
#define    LOTTERY_FAUCET          23

class Lottery : public Poco::Runnable, public AppBaseClass
{
public:
    Lottery(ITNS_TIPBOT * DiscordPtr);
    virtual ~Lottery();

    void                                save();
    void                                load();
    void                                setAccount(Account *);
    iterator                            begin();
    const_iterator                      begin() const;
    const_iterator                      cbegin() const;

    iterator                            end();
    const_iterator                      end() const;
    const_iterator                      cend() const;

    void                                run();

    void                                gameInfo(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me);
    void                                LotteryHelp(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me);
    void                                Jackpot(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me);
    void                                BuyTicket(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me);
    void                                MyTickets(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me);

private:
    ITNS_TIPBOT *                   DiscordPtr;
    bool                            lotterySuspended;
    std::uint64_t                   lastWinningTopBlock;
    Account*                        currentUsrAccount{};
    std::shared_ptr<RPCProc>        LotteryAccount;
    std::vector<struct Command>     Commands;
};
