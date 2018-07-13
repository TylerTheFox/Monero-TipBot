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
#include "Tipbot.h"
#include "Account.h"
#include "AppBaseClass.h"

/*
    Every message insert snowflake into users (this will keep it unqiue and fair) 
    Every 30 minutes add a user to PendingTransfers selected randomy from Users then clear Users.
    Every 15 minutes pop a user from PendingTransfers and pay them the faucet fee.
    This will use a thread for PendingTransfers processing. A user will be reinserted if transfer fails.
*/

/*
    Commands: (ADMIN)
    !disallow id
    !paymentqueuesize
    !roundusersize
*/

class ChatRewards : public AppBaseClass
{
public:
    ChatRewards(TIPBOT * DP);
    virtual ~ChatRewards() = default;

    void                                run(const UserMessage & message);
    void                                save();
    void                                load();
    void                                setAccount(Account *);
    iterator                            begin();
    const_iterator                      begin() const;
    const_iterator                      cbegin() const;

    iterator                            end();
    const_iterator                      end() const;
    const_iterator                      cend() const;
private:
    TIPBOT*                             DiscordPtr;
    std::set<Snowflake>                 Users;
    std::set<DiscordID>                 NotAllowedIDs;
    std::stack<Snowflake>               PendingTransfers;
    std::uint64_t                       lastTimePaymentWasSent;
    std::uint64_t                       lastTimeUserDrawn;
    std::uint64_t                       channel;

    void                                help(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void                                DisallowID(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void                                AllowID(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void                                SetChannel(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void                                PaymentQueueSize(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void                                RoundUserSize(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void                                ToggleChatRewards(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);

    void                                ProcessPendingTransfers();
    bool                                isUserDisallowed(const DiscordID & id);
    bool                                enabled;
    Poco::Logger*                       PLog;
    std::vector<struct Command>         Commands;
};