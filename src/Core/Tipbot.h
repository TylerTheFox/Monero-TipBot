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
#include <unordered_set>
#include "Poco/Checksum.h"
#include "Script.h"
#define FIND_USER_UNKNOWN_USER "Unknown User"

extern const char *aboutStr;
const std::string AllowChannelTypeNames[] =
{
    "Public Channel Only",
    "Direct Message Only"
};

enum class AllowChannelTypes
{
    Error = -3,
    CLI = -2,
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

    template <class Archive>
    void serialize(Archive & ar)
    {
        ar(CEREAL_NVP(id), CEREAL_NVP(id_str), CEREAL_NVP(username), CEREAL_NVP(discriminator));
    }
};

inline bool operator<(const Snowflake &a, const Snowflake &b)
{
    return a.id < b.id;
}

struct UserMessage
{
    enum AllowChannelTypes  ChannelPerm;
    struct Snowflake        User;
    Snowflake               Channel;
    std::string             Message;
    std::vector<Snowflake>  Mentions;

    template <class Archive>
    void serialize(Archive & ar)
    {
        ar(CEREAL_NVP(ChannelPerm), CEREAL_NVP(User), CEREAL_NVP(Channel), CEREAL_NVP(Message), CEREAL_NVP(Mentions));
    }
};

class TIPBOT;
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
struct ExecuteCommand
{
    Poco::Timestamp      time_started;
    const UserMessage&   message;
    const Command&       me;

    bool operator==(const ExecuteCommand &lhs) const
    {
        return (time_started == lhs.time_started
            && &message == &lhs.message
            && &me == &lhs.me);
    }
};

struct ExecuteCommandHash
{
    std::size_t operator () (const ExecuteCommand& p) const
    {
        Poco::Checksum csum;
        csum.update(reinterpret_cast<const char*>(&p), sizeof(ExecuteCommand));
        return csum.checksum();
    }
};
typedef std::unordered_set<ExecuteCommand, ExecuteCommandHash> ExecuteCommandType;

struct CommandPerformance
{
    CommandPerformance() : totalTime(0), calls(0)
    {} 

    std::uint64_t totalTime;
    std::uint64_t calls;

    template <class Archive>
    void serialize(Archive & ar)
    {
        ar(CEREAL_NVP(totalTime), CEREAL_NVP(calls));
    }
};
typedef std::unordered_map<std::string, CommandPerformance> PerformanceMap;

class AppBaseClass;
#define DISCORD_USER_CACHE_FILENAME "DISCORDDATA.json"
#define PERFORMANCE_STATS_FILENAME  "PERFORMANCE.json"

class TIPBOT {
public:
    TIPBOT();
    ~TIPBOT();
    virtual void                                        start() = 0;
    void                                                shutdown();
    const struct DiscordUser &                          findUser(const DiscordID & id);
    static bool                                         isUserAdmin(const UserMessage& message);
    void                                                ProcessCommand(const UserMessage& message);
    void                                                CommandParseError(const UserMessage& message, const struct Command & me);
    static bool                                         isCommandAllowedToBeExecuted(const UserMessage& message, const Command& command);
    static std::string                                  generateHelpText(const std::string & title, const std::vector<Command>& cmds, const UserMessage& message);
    void                                                saveUserList();
    const struct TopTakerStruct                         findTopTaker();
    void                                                AppSave();

    void                                                SendMsg(const UserMessage& data, std::string message);
    void                                                SendDirectMsg(const DiscordID& usr, std::string message);

    std::uint64_t                                       totalFaucetAmount();
    uint8_t                                             getUserLang(const DiscordID & usr);
    void                                                tipbot_init();
    virtual const DiscordUser &                         getUserFromServer(DiscordID user) = 0;
    const PerformanceMap&                               getPerformanceStats();
    const ExecuteCommandType&                           getRunningCommands();
    std::vector<std::shared_ptr<AppBaseClass>>&         getApps();
protected:
    std::map<std::uint64_t, std::set<DiscordUser> >     UserList;
    void                                                loadUserList();

private:
    static bool                                         init;
    Poco::Logger*                                       PLog;
    std::vector<std::shared_ptr<AppBaseClass>>          Apps;    
    ExecuteCommandType                                  runningCommands;
    PerformanceMap                                      AppPerformanceStats;
    Poco::Mutex                                         DispatchMu;
    Script                                              ScriptEng;

    void                                                globalHelpMenu(const UserMessage& message);
    void                                                SaveStats();
    void                                                LoadStats();
    void                                                dispatcher(const UserMessage& message, const struct Command & me, const std::shared_ptr<AppBaseClass> & ptr);
    virtual void                                        broadcastMsg(DiscordID channel, std::string message) = 0;
    virtual void                                        broadcastDirectMsg(DiscordID user, std::string message) = 0;
    virtual void                                        _shutdown() = 0;
};
