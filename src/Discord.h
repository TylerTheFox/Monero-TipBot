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
#include "Poco/NumberParser.h"
#include "types.h"
#include "cereal/cereal.hpp"
#include <set>
#include <map>
#include <vector>
#include <memory>
#include "Poco/Logger.h"
#include "Poco/AutoPtr.h"
#include "Config.h"

extern const char *aboutStr;
const std::string AllowChannelTypeNames[] =
{
    "Public Channel Only",
    "Direct Message Only"
};

enum AllowChannelTypes
{
    Any = -1,
    Public = 0,
    Private = 1
};

struct DiscordUser
{
    DiscordID                   id{};
    std::string                 username;
    std::uint64_t               join_epoch_time{};
    mutable std::uint64_t       faucet_epoch_time{};
    mutable std::uint64_t       total_faucet_itns_donated{};
    mutable std::uint64_t       total_faucet_itns_sent{};
    mutable std::uint64_t       total_itns_given{};
    mutable std::uint64_t       total_itns_recieved{};
    mutable std::uint64_t       total_itns_withdrawn{};
    mutable std::uint8_t        language;

    template <class Archive>
    void save(Archive & ar) const
    {
        ar(CEREAL_NVP(id),
            CEREAL_NVP(username),
            CEREAL_NVP(join_epoch_time),
            CEREAL_NVP(faucet_epoch_time),
            CEREAL_NVP(total_faucet_itns_donated),
            CEREAL_NVP(total_faucet_itns_sent),
            CEREAL_NVP(total_itns_given),
            CEREAL_NVP(total_itns_recieved),
            CEREAL_NVP(total_itns_withdrawn),
            CEREAL_NVP(language)
        );
    }

    template <class Archive>
    void load(Archive & ar)
    {
        ar(CEREAL_NVP(id),
            CEREAL_NVP(username),
            CEREAL_NVP(join_epoch_time),
            CEREAL_NVP(faucet_epoch_time),
            CEREAL_NVP(total_faucet_itns_donated),
            CEREAL_NVP(total_faucet_itns_sent),
            CEREAL_NVP(total_itns_given),
            CEREAL_NVP(total_itns_recieved),
            CEREAL_NVP(total_itns_withdrawn)
        );

        if (GlobalConfig.About.major > 2 || GlobalConfig.About.major >= 2 && GlobalConfig.About.minor > 3)
        {
            ar(CEREAL_NVP(language));
        }
        else language = 0;
    }
}; 

struct TopTakerStruct
{
    struct DiscordUser me;
    std::uint64_t amount;
};

inline bool operator<(const DiscordUser &a, const DiscordUser &b)
{
    return a.id < b.id;
}

struct Snowflake
{
    DiscordID               id;
    std::string             id_str;
    std::string             username;
    std::string             discriminator;
};

struct UserMessage
{
    enum AllowChannelTypes  ChannelPerm;
    struct Snowflake        User;
    Snowflake               Channel;
    std::string             Message;
    std::vector<Snowflake>  Mentions;
};

class AppBaseClass;
#define DISCORD_USER_CACHE_FILENAME "DISCORDDATA.json"
class TIPBOT : public SleepyDiscord::DiscordClient {
public:
    using SleepyDiscord::DiscordClient::DiscordClient;
    ~TIPBOT();

    int                                             getDiscordChannelType(SleepyDiscord::Snowflake<SleepyDiscord::Channel> id);
    std::string                                     getDiscordDMChannel(DiscordID id);

    template<class t>
    static DiscordID                                convertSnowflakeToInt64(t id);
    const struct DiscordUser &                      findUser(const DiscordID & id);
    static bool                                     isUserAdmin(const UserMessage& message);
    void                                            CommandParseError(const UserMessage& message, const struct Command & me);
    static bool                                     isCommandAllowedToBeExecuted(const UserMessage& message, const Command& command);
    static std::string                              generateHelpText(const std::string & title, const std::vector<Command>& cmds, const UserMessage& message);
    void                                            saveUserList();
    const struct TopTakerStruct                     findTopTaker();
    void                                            AppSave();

    std::uint64_t                                   totalFaucetAmount();

    uint8_t                                         getUserLang(const SleepyDiscord::Snowflake<SleepyDiscord::User> & usr);
    uint8_t                                         getUserLang(const DiscordID & usr);

    void                                            onMessage(SleepyDiscord::Message message);
    void                                            onReady(SleepyDiscord::Ready readyData);
private:
    Poco::Logger*                                   PLog;
    std::vector<std::shared_ptr<AppBaseClass>>      Apps;
    std::map<std::uint64_t, std::set<DiscordUser> > UserList;
    SleepyDiscord::User                             BotUser;

    void                                            init();
    void                                            refreshUserList();
    void                                            loadUserList();

    struct UserMessage                              ConvertSleepyDiscordMsg(const SleepyDiscord::Message & message);
};

template<class t>
DiscordID TIPBOT::convertSnowflakeToInt64(t id)
{
    return Poco::NumberParser::parseUnsigned64(static_cast<std::string>(id));
}

struct Command
{
    std::string                                                                         name;
    std::function<void(TIPBOT *, const UserMessage&, const Command &)>                  func;
    std::string                                                                         params;
    bool                                                                                opensWallet;
    bool                                                                                adminTools;
    enum AllowChannelTypes                                                              ChannelPermission;
};

typedef std::vector<struct Command>::iterator       iterator;
typedef std::vector<struct Command>::const_iterator const_iterator;
typedef void(*CommandFunc)(TIPBOT * DiscordPtr, const SleepyDiscord::Message & message, const Command & me);
