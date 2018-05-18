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
#define COIN_CONFIG "Coins/"

std::string coin_config;

void setup()
{
    // Coin Select
    std::ifstream in("coin_config.json");
    if (in.is_open())
    {
        std::cout << "Loading Coin Config to disk...\n";
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
            std::cout << "Saving Coin Config to disk...\n";
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
        std::cout << "Token saved to coin config\n";
    }
}

int main()
{
    while (!GlobalConfig.General.Quitting)
    {
        try
        {
            // Setup routine
            setup();

            // Init RPCMan
            RPCMan.reset(new RPCManager);

            // Load RPCs
            RPCMan->load();

            // Run bot with token.
            TIPBOT client(GlobalConfig.General.discordToken, 2);
            client.init();
            RPCMan->setDiscordPtr(&client);

            // Create RPC threads
            Poco::Thread thread;
            thread.start(*RPCMan);

            client.run();

            GlobalConfig.General.Shutdown = true;
            while (GlobalConfig.General.Threads);

            RPCMan.reset(nullptr);

            // Shutdown Complete!
            GlobalConfig.General.Shutdown = false;
        }
        catch (const Poco::Exception & exp)
        {
            std::cerr << "Poco Error: " << exp.what() << "\n";
            return -1;
        }
        catch (AppGeneralException & exp)
        {
            GlobalConfig.General.Shutdown = true;
            return -2;
        }
        catch (const SleepyDiscord::ErrorCode & exp)
        {
            GlobalConfig.General.Shutdown = true;
            return -3;
        }
    }
    std::cout << "Bot safely shutdown!";
    std::cin.get();
    return 0;
}
