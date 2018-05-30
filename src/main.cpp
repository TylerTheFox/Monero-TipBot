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
#include <string>
#include <iostream>
#include "Poco/File.h"
#include <fstream>
#include "RPCManager.h"
#include "Poco/Thread.h"
#include "RPCException.h"
#include "Config.h"
#include "Faucet.h"
#include "Lottery.h"
#include "Poco/File.h"

#include "Poco/Logger.h"
#include "Poco/SplitterChannel.h"
#include "Poco/ConsoleChannel.h"
#include "Poco/SimpleFileChannel.h"
#include "Poco/AutoPtr.h"
#include "Poco/FormattingChannel.h"
#include "Poco/PatternFormatter.h"
#define COIN_CONFIG "Coins/"

std::string coin_config;

void setupLogging()
{
    Poco::AutoPtr<Poco::ConsoleChannel> pCons(new Poco::ConsoleChannel);
    Poco::AutoPtr<Poco::SimpleFileChannel> pFile(new Poco::SimpleFileChannel("tipbot.log"));
    Poco::AutoPtr<Poco::PatternFormatter> pPF(new Poco::PatternFormatter);
    Poco::AutoPtr<Poco::SplitterChannel> pSplitter(new Poco::SplitterChannel);
    pFile->setProperty("rotation", "2 K");
    pSplitter->addChannel(pCons);
    pSplitter->addChannel(pFile);
    pPF->setProperty("pattern", "%Y-%m-%d %H:%M:%S [%p] %s: %t");
    Poco::AutoPtr<Poco::FormattingChannel> pFC(new Poco::FormattingChannel(pPF, pSplitter));
    Poco::Logger::root().setChannel(pFC);
}

void setup()
{
    auto & logger = Poco::Logger::get("Setup");

    // Coin Select
    std::ifstream in("coin_config.json");
    if (in.is_open())
    {
        logger.information("Loading Coin Config to disk...");
        {
            cereal::JSONInputArchive ar(in);
            ar(CEREAL_NVP(coin_config));
        }
        in.close();
    }

    if (coin_config.empty())
    {
        std::cout << "Welcome to Tipbot!\n"
            << "Created by Brandan Tyler Lasley\n";

        Poco::File coindir(COIN_CONFIG);
        std::vector<Poco::File> config_files;
        coindir.list(config_files);

        std::map<unsigned int, std::string> config_map;
        unsigned int selected = 0;
        unsigned int i = 0;
        std::cout << "Select Config: \n";
        for (auto file : config_files)
        {
            std::cout << "[" << ++i << "]" << " " << file.path() << "\n";
            config_map[i] = file.path();
        }

        while (selected == 0 || selected > config_map.size())
        {
            std::cout << "Select: ";
            std::cin >> selected;
        }
        coin_config = config_map[selected];

        std::ofstream out("coin_config.json", std::ios::trunc);
        if (out.is_open())
        {
            logger.information("Saving Coin Config to disk...");
            {
                cereal::JSONOutputArchive ar(out);
                ar(CEREAL_NVP(coin_config));
            }
            out.close();
        }
    }

    // Load Config
    GlobalConfig.load_config(coin_config);

    // Token setup.
    if (GlobalConfig.General.discordToken.empty())
    {
        std::cout << "Please enter Discord Token: ";
        std::cin >> GlobalConfig.General.discordToken;
        GlobalConfig.save_config();
        logger.information("Token saved to coin config");
    }
}

int main()
{
    while (!GlobalConfig.General.Quitting)
    {
        // Enable logging.
        setupLogging();
        auto & logger = Poco::Logger::get("Main");
        logger.information("Tipbot starting up...");

        try
        {
            // Setup routine
            setup();

            // Init RPCMan
            RPCMan.reset(new RPCManager);

            // Load RPCs
            RPCMan->load();

            // Run bot with token.
            TIPBOT client(GlobalConfig.General.discordToken);
            RPCMan->setDiscordPtr(&client);

            // Create RPC threads
            Poco::Thread thread;
            thread.start(*RPCMan);
            client.run();
        }
        catch (const websocketpp::exception & err)
        {
            logger.error("websocketpp Code: %?i Message: %s", err.code(), err.m_msg);
        }
        catch (const Poco::Exception & exp)
        {
            logger.error("Poco Error:  %s", std::string(exp.what()));
        }
        catch (AppGeneralException & exp)
        {
            logger.error("App Error:  %s", std::string(exp.what()));

        }
        catch (const SleepyDiscord::ErrorCode & exp)
        {
            logger.error("Discord Error: %?i", exp);
        }

        logger.information("Tipbot shutting down...");

        RPCMan.reset(nullptr);

        // Upgrade save file
        if (VERSION_MAJOR != GlobalConfig.About.major || VERSION_MINOR != GlobalConfig.About.minor)
        {
            logger.information("Upgrading Save file...");
            GlobalConfig.About.major = VERSION_MAJOR;
            GlobalConfig.About.minor = VERSION_MINOR;
            GlobalConfig.save_config();
        }

        // Shutdown Complete!
        Poco::Logger::shutdown();
        GlobalConfig.General.Shutdown = false;

        logger.information("Tipbot shutdown complete...");
    }
    return 0;
}
