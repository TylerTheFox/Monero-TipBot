#include "CLI.h"
#include "Config.h"
#include "Poco/Thread.h"
#include "RPCManager.h"
#include <Poco/StringTokenizer.h>

CLI::CLI(TIPBOT * dptr)
{
    PLog = &Poco::Logger::get("CLI");

    DiscordPtr = dptr;

    // Create CLI thread
    std::thread t1(&CLI::cli_main, this);
    t1.detach();
}

CLI::~CLI()
{

}

void CLI::cli_main()
{
    GlobalConfig.General.Threads++;

    std::string buffer;
    while (!GlobalConfig.General.Shutdown)
    {
        std::getline(std::cin, buffer);
        if (!buffer.empty())
        {
            PLog->information(buffer);
            DiscordPtr->ProcessCommand(generateUsrMsg(buffer));
        }
    }

    GlobalConfig.General.Threads--;
}


UserMessage CLI::generateUsrMsg(std::string msg)
{
    UserMessage ret = {};

    ret.Message = msg;
    ret.ChannelPerm = AllowChannelTypes::CLI;
    ret.User.id = RPCMan->getBotDiscordID();
    ret.User.id_str = Poco::format("%?i", ret.User.id);
    ret.User.username = "CLI";
    ret.User.discriminator = "CLI";


    // Generate Mentions (By ID)
    try
    {
        Poco::StringTokenizer cmd(ret.Message, " ");
        std::string usr;
        Snowflake newUsr;
        for (auto elm : cmd)
        {
            if (!elm.empty() && elm.at(0) == '@')
            {
                usr = Poco::replace(elm, "@", "");
                const auto usrid = Poco::NumberParser::parseUnsigned64(usr);
                newUsr.id = usrid;
                newUsr.id_str = usr;
                auto disUsr = DiscordPtr->findUser(newUsr.id);
                if (disUsr.username != FIND_USER_UNKNOWN_USER)
                {
                    newUsr.discriminator = "????";
                    newUsr.username = disUsr.username;
                    ret.Mentions.emplace_back(newUsr);
                }
            }
        }
    }
    catch (Poco::Exception exp)
    {
        // We don't care, intentionally left blank.
    }

    return ret;
}

void CLI::save()
{
}

void CLI::load()
{
}

void CLI::setAccount(Account* acc)
{

}

iterator CLI::begin()
{
    return Commands.begin();
}

const_iterator CLI::begin() const
{
    return Commands.begin();
}

const_iterator CLI::cbegin() const
{
    return Commands.cbegin();
}

iterator CLI::end()
{
    return Commands.end();
}

const_iterator CLI::end() const
{
    return Commands.end();
}

const_iterator CLI::cend() const
{
    return Commands.cend();
}