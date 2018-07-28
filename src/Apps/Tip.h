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
#pragma once
#include "../Core/Tipbot.h"
#include "../Core/AppBaseClass.h"
#include "../Core/Account.h"
#include <vector>
#include "Poco/Timestamp.h"
class TIPBOT;

struct Settings
{
    bool giveAllowed;
    bool withdrawAllowed;
};

class Tip : public AppBaseClass
{
public:
    Tip();
    ~Tip() = default;

    void                            save();
    void                            load();
    iterator                        begin();
    const_iterator                  begin() const;
    const_iterator                  cbegin() const;

    iterator                        end();
    const_iterator                  end() const;
    const_iterator                  cend() const;
    void                            setAccount(Account *);

    void                            Help(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void                            Balance(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void                            MyAddress(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void                            History(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void                            Withdraw(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void                            WithdrawAll(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void                            Give(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void                            GiveAll(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void                            About(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void                            BlockHeight(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void                            RestartWallet(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void                            ListLanguages(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void                            SelectLanguage(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);

    // Admin
    void                            ToggleWithdraw(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void                            ToggleGive(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void                            RescanAllWallets(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void                            TotalBalance(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void                            SaveWallets(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void                            RestartFaucetWallet(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void                            SoftRestartBot(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void                            Shutdown(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void                            RPCStatus(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void                            WhoIs(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void                            PerformanceData(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void                            Executing(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);
    void                            UpTime(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me);

    private:
    Poco::Timestamp                 start;
    Settings                        globalSettings{};
    std::vector<struct Command>     Commands;
    Account*                        MyAccount;
};
