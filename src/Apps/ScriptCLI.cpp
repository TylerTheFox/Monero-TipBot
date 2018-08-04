#include "ScriptCLI.h"
#include "Poco/StringTokenizer.h"

#define CLASS_RESOLUTION(x) std::bind(&ScriptCLI::x, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
ScriptCLI::ScriptCLI(TIPBOT * DPTR, Script * scrptr) : AppBaseClass(DPTR), _scrptr(scrptr)
{
    setName("ScriptCLI");
    Commands =
    {
        // User Commands 
        // Command                  Function                                      Params                              Wallet  Admin   Allowed Channel
        { "!script_help",           CLASS_RESOLUTION(script_help),                "",                                 false,  true,   AllowChannelTypes::CLI },
        { "!script_start",          CLASS_RESOLUTION(script_start),               "",                                 false,  true,   AllowChannelTypes::CLI },
        { "!script_shutdown",       CLASS_RESOLUTION(script_shutdown),            "",                                 false,  true,   AllowChannelTypes::CLI },
        { "!script_restart",        CLASS_RESOLUTION(script_restart),             "",                                 false,  true,   AllowChannelTypes::CLI },
        { "!script_count",          CLASS_RESOLUTION(script_count),               "",                                 false,  true,   AllowChannelTypes::CLI },
        { "!script_list",           CLASS_RESOLUTION(script_list),                "",                                 false,  true,   AllowChannelTypes::CLI },
        { "!script_shutdown_all",   CLASS_RESOLUTION(script_shutdown_all),        "",                                 false,  true,   AllowChannelTypes::CLI },
    };

    setHelpCommand(Commands[0]);
}

void ScriptCLI::script_help(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me)
{
    const auto helpStr = TIPBOT::generateHelpText("Scripts Menu", Commands, message);
    DiscordPtr->SendMsg(message, helpStr);
}


void ScriptCLI::script_start(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me)
{
    Poco::StringTokenizer cmd(message.Message, " ");

    if (cmd.count() != 2)
        DiscordPtr->CommandParseError(message, me);
    else
    {
        const auto& script = cmd[1];
        if (_scrptr->add_script(script))
            DiscordPtr->SendMsg(message, "Script Added Successfully!");
        else
            DiscordPtr->SendMsg(message, "Script Failed to load!");
    }
}

void ScriptCLI::script_shutdown(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me)
{
    Poco::StringTokenizer cmd(message.Message, " ");

    if (cmd.count() != 2)
        DiscordPtr->CommandParseError(message, me);
    else
    {
        const auto& script = cmd[1];
        _scrptr->remove_script(script);
        DiscordPtr->SendMsg(message, "Script Removed Successfully!");
    }
}

void ScriptCLI::script_restart(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me)
{
    Poco::StringTokenizer cmd(message.Message, " ");

    if (cmd.count() != 2)
        DiscordPtr->CommandParseError(message, me);
    else
    {
        const auto& script = cmd[1];
        _scrptr->add_script(script);
        DiscordPtr->SendMsg(message, "Script Restarted Successfully!");
    }
}

void ScriptCLI::script_count(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me)
{
    DiscordPtr->SendMsg(message, Poco::format("There are a total of %?i scripts running!", _scrptr->count()));;
}

void ScriptCLI::script_list(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me)
{
    DiscordPtr->SendMsg(message, "Loaded Scripts:");
    const auto & scrs = _scrptr->getScripts();
    for (const auto & scr : scrs)
    {
        DiscordPtr->SendMsg(message, scr.path);
    }
}

void ScriptCLI::script_shutdown_all(TIPBOT * DiscordPtr, const UserMessage& message, const struct Command & me)
{
    _scrptr->clearAll();
    DiscordPtr->SendMsg(message, "All scripts shutdown!");
}
