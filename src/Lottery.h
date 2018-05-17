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

class Lottery : public Poco::Runnable, public AppBaseClass
{
public:
    Lottery(TIPBOT * DiscordPtr);
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

    void                                gameInfo(TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me) const;
    void                                LotteryHelp(TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me) const;
    void                                Jackpot(TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me) const;
    void                                BuyTicket(TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me) const;
    void                                MyTickets(TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me) const;

    void                                ToggleLotterySuspend(TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me);
private:
    TIPBOT *                   DiscordPtr;
    bool                            lotterySuspended;
    std::uint64_t                   lastWinningTopBlock = 0;
    Account*                        currentUsrAccount{};
    std::shared_ptr<RPCProc>        LotteryAccount;
    std::vector<struct Command>     Commands;
};
