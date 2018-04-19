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
#include "sleepy_discord/websocketpp_websocket.h"

class ITNS_TIPBOT;

enum AllowChannelTypes
{
    Any = -1,
    Public = 0,
    Private = 1
};

struct Settings
{
    bool commandsAllowed;
    bool giveAllowed;
    bool withdrawAllowed;
};

struct Command
{
    std::string             name;
    void *                  func;
    std::string             params;
    bool                    opensWallet;
    bool                    adminTools;
    enum AllowChannelTypes  ChannelPermission;
};
typedef void(*CommandFunc)(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const Command & me);

namespace DiscordCommands
{
    void ProcessCommand(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message);
    void CommandParseError(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message& message, const struct Command & me);

    void Help(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me);
    void Balance(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me);
    void MyAddress(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me);
    void History(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me);
    void Withdraw(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me);
    void WithdrawAll(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me);
    void Give(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me);
    void GiveAll(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me);
    void About(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me);
    void BlockHeight(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me);

    // Admin
    void ToggleWithdrawAllowed(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me);
    void ToggleGiveAllowed(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me);
    void ToggleCommandsAllowed(ITNS_TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const struct Command & me);

    bool isUserAdmin(ITNS_TIPBOT* DiscordPtr, const SleepyDiscord::Message & message);
};
