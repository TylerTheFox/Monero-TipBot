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
#include "Discord.h"
#include "Poco/NumberParser.h"
#include "DiscordCommands.h"
#include "Poco/JSON/Parser.h"

int ITNS_TIPBOT::getDiscordChannelType(SleepyDiscord::Snowflake<SleepyDiscord::Channel> id)
{
    Poco::JSON::Parser          parser;
    Poco::JSON::Object::Ptr     object;
    std::string                 clientID;
    auto response = getChannel(id);
    object = parser.parse(response.text).extract<Poco::JSON::Object::Ptr>();
    return object->getValue<int>("type");
}

std::string ITNS_TIPBOT::getDiscordDMChannel(DiscordID id)
{
    Poco::JSON::Parser      parser;
    Poco::JSON::Object::Ptr object;
    std::string             clientID;
    auto response   =    createDirectMessageChannel(Poco::format("%Lu", id));
    object          =    parser.parse(response.text).extract<Poco::JSON::Object::Ptr>();
    return object->getValue<std::string>("id");
}

DiscordID ITNS_TIPBOT::convertSnowflakeToInt64(SleepyDiscord::Snowflake<SleepyDiscord::User> id)
{
    return Poco::NumberParser::parseUnsigned64(static_cast<std::string>(id));
}

void ITNS_TIPBOT::onMessage(SleepyDiscord::Message message)
{
    DiscordCommands::ProcessCommand(this, message);
}
