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

/*
 *
 * Lottery opens at MIDNIGHT Saturdays
 * Lottery closes at 6 PM PDT every Friday
 * Winners announced 9 PM PDT every Friday
 * 5% of the winnings goes to the Faucet.
 *  
 */
#pragma once
#include "../Core/Tipbot.h"
#include "../Core/Account.h"
#include "../Core/AppBaseClass.h"
#include "../Core/RPCManager.h"
#include "Poco/Logger.h"
#include "Poco/AutoPtr.h"
#define    LOTTERY_USER            "LOTTERY" // Wallet
#define    LOTTERY_SAVE_FILE       "LOTTERY.JSON"

class Lottery : public Poco::Runnable, public AppBaseClass
{
public:
    Lottery(TIPBOT * DPTR);
    virtual ~Lottery();

    void                                save();
    void                                load();
    void                                setAccount(Account *);

    void                                run();

    void                                gameInfo(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me) const;
    void                                LotteryHelp(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me) const;
    void                                Jackpot(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me) const;
    void                                BuyTicket(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me) const;
    void                                MyTickets(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me) const;
    void                                LotteryWon(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me) const;

    void                                lastWinner(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me) const;
    void                                ToggleLotterySuspend(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
private:
    TIPBOT *                        DiscordPtr;
    std::uint64_t                   lastWinningTopBlock = 0;
    Account*                        currentUsrAccount{};
    std::shared_ptr<RPCProc>        LotteryAccount;
    DiscordID                       prevWinner;
    bool rewardGivenout = false;
    bool sweepComplete = false;
    bool noWinner = false;
};
