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
#ifndef NO_CHAISCRIPT
#include "Tipbot.h"
#include "ScriptDefs.h"
#include "Script.h"
#include "Poco/Thread.h"
#include "RPCManager.h"
#include "RPC.h"
#include "Account.h"
#include "Config.h"
#include "Language.h"
#include "Util.h"
#include "Poco/StringTokenizer.h"
#include "Poco/URI.h"
#include <vector>
#include <string>
#include "Poco/NumberParser.h"
#include "Server.h"

ScriptDefs::ScriptDefs()
{
    PLog = &Poco::Logger::get("Script");
    m.reset(new chaiscript::Module());

    // Init Module
    core_datatypes();
    core_datatypes_impli();
    class_functions();
    core_functions();
    modern_vars();
}

chaiscript::ModulePtr & ScriptDefs::getModule()
{
    return m;
}

void ScriptDefs::core_datatypes() const
{
    MODULE_ADD_TYPE_FULL_EASY(RPCManager);      // Done
    MODULE_ADD_TYPE_BASIC_EASY(TIPBOT);         // Done
    MODULE_ADD_TYPE_FULL_EASY(RPC);             // Done
    MODULE_ADD_TYPE_FULL_EASY(BalanceRet);      // Done
    MODULE_ADD_TYPE_FULL_EASY(TransferRet);     // Done
    MODULE_ADD_TYPE_FULL_EASY(TransferItem);    // Done
    MODULE_ADD_TYPE_FULL_EASY(TransferList);    // Done
    MODULE_ADD_TYPE_FULL_EASY(Account);         // Done
    MODULE_ADD_TYPE_FULL_EASY(UserMessage);     // Done
    MODULE_ADD_TYPE_FULL_EASY(Command);         // Done
    MODULE_ADD_TYPE_FULL_EASY(Snowflake);       // Done
    MODULE_ADD_TYPE_FULL_EASY(TopTakerStruct);  // Done
    MODULE_ADD_TYPE_FULL_EASY(DiscordUser);     // Done
    MODULE_ADD_TYPE_FULL_EASY(AboutConfig);     // Done
    MODULE_ADD_TYPE_FULL_EASY(GeneralConfig);   // Done
    MODULE_ADD_TYPE_FULL_EASY(RPCConfig);       // Done 
    MODULE_ADD_TYPE_FULL_EASY(RPCManagerConfig);// Done
    MODULE_ADD_TYPE_FULL_EASY(FacuetConfig);    // Done
    MODULE_ADD_TYPE_FULL_EASY(LotteryConfig);   // Done
    MODULE_ADD_TYPE_FULL_EASY(ChatRewardsConfig);// Done
    MODULE_ADD_TYPE_FULL_EASY(AppConfig);       // Done
    MODULE_ADD_TYPE_FULL_EASY(Lang);            // Done
    MODULE_ADD_TYPE_FULL_EASY(LanguageSelect);  // Done
    MODULE_ADD_TYPE_FULL_EASY(RPCProc);         // Done
    MODULE_ADD_TYPE_BASIC_EASY(Server);         // Done
    m->add(chaiscript::user_type<Poco::Net::SocketStream>(), "SocketStream");
}

void ScriptDefs::core_datatypes_impli() const
{
    ////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////   BalanceRet            //////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////
    MODULE_ADD(BalanceRet::Balance, "Balance");
    MODULE_ADD(BalanceRet::UnlockedBalance, "UnlockedBalance");

    ////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////   TransferRet            /////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////
    MODULE_ADD(TransferRet::fee, "fee");
    MODULE_ADD(TransferRet::tx_hash, "tx_hash");
    MODULE_ADD(TransferRet::tx_key, "tx_key");

    ////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////   TransferItem            ////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////
    MODULE_ADD(TransferItem::amount, "amount");
    MODULE_ADD(TransferItem::block_height, "block_height");
    MODULE_ADD(TransferItem::payment_id, "payment_id");
    MODULE_ADD(TransferItem::tx_hash, "tx_hash");

    ////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////   TransferList            ////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////
    MODULE_ADD(TransferList::tx_in, "tx_in");
    MODULE_ADD(TransferList::tx_out, "tx_out");

    ////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////   RPC            /////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////
    chaiscript::utility::add_class<RPC>(*m,
        "RPC",
        {
            chaiscript::constructor<RPC()>(),
            chaiscript::constructor<RPC(const RPC & obj)>(),
        },
        {
            { chaiscript::fun(&RPC::open), "open" },
            { chaiscript::fun(&RPC::getBalance), "getBalance" },
            { chaiscript::fun(&RPC::getAddress), "getAddress" },
            { chaiscript::fun(&RPC::getBlockHeight), "getBlockHeight" },
            { chaiscript::fun(static_cast<TransferRet(RPC::*)(const std::string &, std::uint64_t, const std::string &, int) const>(&RPC::tranfer)), "tranfer" },
            { chaiscript::fun(static_cast<TransferRet(RPC::*)(const std::string &, const std::string &, int) const>(&RPC::sweepAll)), "sweepAll" },
            { chaiscript::fun(&RPC::getTransfers), "getTransfers" },
            { chaiscript::fun(&RPC::createWallet), "createWallet" },
            { chaiscript::fun(&RPC::openWallet), "openWallet" },
            { chaiscript::fun(&RPC::stopWallet), "stopWallet" },
            { chaiscript::fun(&RPC::store), "store" },
            { chaiscript::fun(&RPC::setTXNote), "setTXNote" },
            { chaiscript::fun(&RPC::getTXNote), "getTXNote" },
            { chaiscript::fun(&RPC::getPort), "getPort" },
            { chaiscript::fun(&RPC::operator=), "=" }
        }
        );

    ////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////   Account         ////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////
    chaiscript::utility::add_class<Account>(*m,
        "Account",
        {
            chaiscript::constructor<Account()>(),
            chaiscript::constructor<Account(const Account & obj)>(),
        },
            {
                { chaiscript::fun(&Account::open), "open" },
                { chaiscript::fun(&Account::getBalance), "getBalance" },
                { chaiscript::fun(&Account::getBlockHeight), "getBlockHeight" },
                { chaiscript::fun(&Account::getDiscordID), "getDiscordID" },
                { chaiscript::fun(&Account::getMyAddress), "getMyAddress" },
                { chaiscript::fun(&Account::getTransactions), "getTransactions" },
                { chaiscript::fun(&Account::getUnlockedBalance), "getUnlockedBalance" },
                { chaiscript::fun(&Account::resyncAccount), "resyncAccount" },
                { chaiscript::fun(&Account::transferAllMoneyToAddress), "transferAllMoneyToAddress" },
                { chaiscript::fun(&Account::transferAllMoneytoAnotherDiscordUser), "transferAllMoneytoAnotherDiscordUser" },
                { chaiscript::fun(&Account::transferMoneyToAddress), "transferMoneyToAddress" },
                { chaiscript::fun(&Account::transferMoneytoAnotherDiscordUser), "transferMoneytoAnotherDiscordUser" },
                { chaiscript::fun(&Account::operator=), "=" }
            }
        );

    // Static
    MODULE_ADD(Account::getWalletAddress, "getWalletAddress");

    ////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////   Tipbot         /////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////
    chaiscript::utility::add_class<TIPBOT>(*m,
        "Tipbot",
        {
        },
            {
            { chaiscript::fun(&TIPBOT::AppSave), "AppSave" },
            { chaiscript::fun(&TIPBOT::CommandParseError), "CommandParseError" },
            { chaiscript::fun(&TIPBOT::findTopTaker), "findTopTaker" },
            { chaiscript::fun(&TIPBOT::findUser), "findUser" },
            { chaiscript::fun(&TIPBOT::getPerformanceStats), "getPerformanceStats" },
            { chaiscript::fun(&TIPBOT::getRunningCommands), "getRunningCommands" },
            { chaiscript::fun(&TIPBOT::getUserFromServer), "getUserFromServer" },
            { chaiscript::fun(&TIPBOT::getUserLang), "getUserLang" },
            { chaiscript::fun(&TIPBOT::ProcessCommand), "ProcessCommand" },
            { chaiscript::fun(&TIPBOT::saveUserList), "saveUserList" },
            { chaiscript::fun(&TIPBOT::SendDirectMsg), "SendDirectMsg" },
            { chaiscript::fun(&TIPBOT::SendMsg), "SendMsg" },
            { chaiscript::fun(&TIPBOT::shutdown), "shutdown" },
            { chaiscript::fun(&TIPBOT::start), "start" },
            { chaiscript::fun(&TIPBOT::tipbot_init), "tipbot_init" },
            { chaiscript::fun(&TIPBOT::totalFaucetAmount), "totalFaucetAmount" }
        }
        );

    // Static
    MODULE_ADD(TIPBOT::isUserAdmin, "isUserAdmin");
    MODULE_ADD(TIPBOT::isCommandAllowedToBeExecuted, "isCommandAllowedToBeExecuted");
    MODULE_ADD(TIPBOT::generateHelpText, "generateHelpText");

    ////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////   RPCManager         /////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////
    chaiscript::utility::add_class<RPCManager>(*m,
        "RPCManager",
        {
        },
        {
            { chaiscript::fun(&RPCManager::getAccount), "getAccount" },
            { chaiscript::fun(&RPCManager::getBotDiscordID), "getBotDiscordID" },
            { chaiscript::fun(&RPCManager::getRPC), "getRPC" },
            { chaiscript::fun(&RPCManager::getTimeStarted), "getTimeStarted" },
            { chaiscript::fun(&RPCManager::getTotalBalance), "getTotalBalance" },
            { chaiscript::fun(&RPCManager::getTransfers), "getTransfers" },
            { chaiscript::fun(&RPCManager::load), "load" },
            { chaiscript::fun(&RPCManager::rescanAll), "rescanAll" },
            { chaiscript::fun(&RPCManager::setBotUser), "setBotUser" },
            { chaiscript::fun(&RPCManager::setDiscordPtr), "setDiscordPtr" },
            { chaiscript::fun(&RPCManager::status), "status" },
            { chaiscript::fun(&RPCManager::waitForRPCToRespond), "waitForRPCToRespond" }
        }
        );

    // Static
    MODULE_ADD(RPCManager::manuallyCreateRPC, "manuallyCreateRPC");
    MODULE_ADD(RPCManager::getGlobalBotRPC, "getGlobalBotRPC");
    MODULE_ADD(RPCManager::getGlobalBotAccount, "getGlobalBotAccount");

    ////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////   LanguageSelect         /////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////
    chaiscript::utility::add_class<LanguageSelect>(*m,
        "LanguageSelect",
        {
            chaiscript::constructor<LanguageSelect()>(),
        },
        {
            { chaiscript::fun(&LanguageSelect::getLanguages), "getLanguages" },
            { chaiscript::fun(&LanguageSelect::getString), "getString" },
            { chaiscript::fun(&LanguageSelect::size), "size" }
        }
        );

    ////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////   UserMessage            /////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////
    MODULE_ADD(UserMessage::ChannelPerm, "ChannelPerm");
    MODULE_ADD(UserMessage::User, "User");
    MODULE_ADD(UserMessage::Channel, "Channel");
    MODULE_ADD(UserMessage::Message, "Message");
    MODULE_ADD(UserMessage::Mentions, "Mentions");

    ////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////   Command            /////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////
    MODULE_ADD(Command::adminTools, "adminTools");
    MODULE_ADD(Command::ChannelPermission, "ChannelPermission");
    MODULE_ADD(Command::func, "func");
    MODULE_ADD(Command::name, "name");
    MODULE_ADD(Command::opensWallet, "opensWallet");
    MODULE_ADD(Command::params, "params");

    ////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////   Snowflake            ///////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////
    MODULE_ADD(Snowflake::discriminator, "discriminator");
    MODULE_ADD(Snowflake::id, "id");
    MODULE_ADD(Snowflake::id_str, "id_str");
    MODULE_ADD(Snowflake::username, "username");

    ////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////   TopTakerStruct            //////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////
    MODULE_ADD(TopTakerStruct::amount, "amount");
    MODULE_ADD(TopTakerStruct::me, "me");

    ////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////   DiscordUser            /////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////
    MODULE_ADD(DiscordUser::faucet_epoch_time, "faucet_epoch_time");
    MODULE_ADD(DiscordUser::id, "id");
    MODULE_ADD(DiscordUser::join_epoch_time, "join_epoch_time");
    MODULE_ADD(DiscordUser::language, "language");
    MODULE_ADD(DiscordUser::total_faucet_itns_donated, "total_faucet_itns_donated");
    MODULE_ADD(DiscordUser::total_faucet_itns_sent, "total_faucet_itns_sent");
    MODULE_ADD(DiscordUser::total_itns_given, "total_itns_given");
    MODULE_ADD(DiscordUser::total_itns_recieved, "total_itns_recieved");
    MODULE_ADD(DiscordUser::total_itns_withdrawn, "total_itns_withdrawn");
    MODULE_ADD(DiscordUser::username, "username");

    ////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////   AboutConfig            /////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////
    MODULE_ADD(AboutConfig::major, "major");
    MODULE_ADD(AboutConfig::minor, "minor");

    ////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////   GeneralConfig            ///////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////
    MODULE_ADD(GeneralConfig::Admins, "Admins");
    MODULE_ADD(GeneralConfig::discordToken, "discordToken");
    MODULE_ADD(GeneralConfig::Quitting, "Quitting");
    MODULE_ADD(GeneralConfig::Shutdown, "Shutdown");
    MODULE_ADD(GeneralConfig::Threads, "Threads");

    ////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////   RPCConfig            ///////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////
    MODULE_ADD(RPCConfig::address_length, "address_length");
    MODULE_ADD(RPCConfig::coin_abbv, "coin_abbv");
    MODULE_ADD(RPCConfig::coin_offset, "coin_offset");
    MODULE_ADD(RPCConfig::daemon_hostname, "daemon_hostname");
    MODULE_ADD(RPCConfig::filename, "filename");
    MODULE_ADD(RPCConfig::hostname, "hostname");
    MODULE_ADD(RPCConfig::json_uri, "json_uri");
    MODULE_ADD(RPCConfig::mixin, "mixin");
    MODULE_ADD(RPCConfig::use_test_net, "use_test_net");
    MODULE_ADD(RPCConfig::wallet_path, "wallet_path");

    ////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////   RPCManagerConfig            ////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////
    MODULE_ADD(RPCManagerConfig::blockchain_save_time, "blockchain_save_time");
    MODULE_ADD(RPCManagerConfig::error_giveup, "error_giveup");
    MODULE_ADD(RPCManagerConfig::max_rpc_limit, "max_rpc_limit");
    MODULE_ADD(RPCManagerConfig::search_for_new_transactions_time, "search_for_new_transactions_time");
    MODULE_ADD(RPCManagerConfig::starting_port_number, "starting_port_number");
    MODULE_ADD(RPCManagerConfig::wallets_save_time, "wallets_save_time");
    MODULE_ADD(RPCManagerConfig::wallet_watchdog_time, "wallet_watchdog_time");

    ////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////   FacuetConfig            ////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////
    MODULE_ADD(FacuetConfig::min_discord_account, "min_discord_account");
    MODULE_ADD(FacuetConfig::percentage_allowance, "percentage_allowance");
    MODULE_ADD(FacuetConfig::timeout, "timeout");

    ////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////   LotteryConfig            ///////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////
    MODULE_ADD(LotteryConfig::close, "close");
    MODULE_ADD(LotteryConfig::day, "day");
    MODULE_ADD(LotteryConfig::donation_percent, "donation_percent");
    MODULE_ADD(LotteryConfig::faucet, "faucet");
    MODULE_ADD(LotteryConfig::no_winner_chance, "no_winner_chance");
    MODULE_ADD(LotteryConfig::pick, "pick");
    MODULE_ADD(LotteryConfig::ticket_cost, "ticket_cost");

    ////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////   ChatRewardsConfig            ///////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////
    MODULE_ADD(ChatRewardsConfig::next_drawing_time, "next_drawing_time");
    MODULE_ADD(ChatRewardsConfig::next_payment_time, "next_payment_time");

    ////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////   LotteryConfig            ///////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////
    MODULE_ADD(AppConfig::About, "About");
    MODULE_ADD(AppConfig::ChatRewards, "ChatRewards");
    MODULE_ADD(AppConfig::Faucet, "Faucet");
    MODULE_ADD(AppConfig::General, "General");
    MODULE_ADD(AppConfig::Lottery, "Lottery");
    MODULE_ADD(AppConfig::RPC, "RPC");
    MODULE_ADD(AppConfig::RPCManager, "RPCManager");

    ////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////   Lang            ////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////
    MODULE_ADD(Lang::LangMap, "LangMap");
    MODULE_ADD(Lang::LangName, "LangName");

    ////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////   Util            ////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////
    MODULE_ADD(Util::getWalletStrFromIID, "getWalletStrFromIID");
    MODULE_ADD(Util::parseQuotedString, "parseQuotedString");
    MODULE_ADD(Util::send_http_post, "send_http_post");
    MODULE_ADD(Util::write_data_to_file, "write_data_to_file");
    MODULE_ADD(Util::read_data_from_file, "read_data_from_file");

    ////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////   RPCProc            /////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////
    MODULE_ADD(RPCProc::MyAccount, "MyAccount");
    MODULE_ADD(RPCProc::MyRPC, "MyRPC");
    MODULE_ADD(RPCProc::pid, "pid");
    MODULE_ADD(RPCProc::RPCFail, "RPCFail");
    MODULE_ADD(RPCProc::timestamp, "timestamp");
    MODULE_ADD(RPCProc::Transactions, "Transactions");

    ////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////   Server         /////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////
    chaiscript::utility::add_class<Server>(*m,
        "Server",
        {
            chaiscript::constructor<Server(unsigned short port)>(),
        },
        {
            { chaiscript::fun(&Server::start), "start" },
            { chaiscript::fun(&Server::stop), "stop" },
            { chaiscript::fun(&Server::setOnConnectFunc), "setOnConnectFunc" },
            { chaiscript::fun(&Server::write), "write" }
        }
        );
}

void ScriptDefs::class_functions() const
{
}

void ScriptDefs::core_functions()
{
    MODULE_ADD(Poco::Thread::sleep, "sleep");
    MODULE_ADD(Poco::URI::encode, "uri_encode");
    MODULE_ADD(Poco::URI::decode, "uri_decode");
    MODULE_ADD(Poco::NumberParser::tryParse, "tryParse");
    MODULE_ADD(Poco::NumberParser::tryParse64, "tryParse64");
    MODULE_ADD(Poco::NumberParser::tryParseBool, "tryParseBool");
    MODULE_ADD(Poco::NumberParser::tryParseFloat, "tryParseFloat");
    MODULE_ADD(Poco::NumberParser::tryParseHex, "tryParseHex");
    MODULE_ADD(Poco::NumberParser::tryParseHex64, "tryParseHex64");
    MODULE_ADD(Poco::NumberParser::tryParseOct, "tryParseOct");
    MODULE_ADD(Poco::NumberParser::tryParseOct64, "tryParseOct64");
    MODULE_ADD(Poco::NumberParser::tryParseUnsigned, "tryParseUnsigned");
    MODULE_ADD(Poco::NumberParser::tryParseUnsigned64, "tryParseUnsigned64");
    MODULE_ADD_LAMBDA(std::function<void(const std::string &)>([&](const std::string & msg) { PLog->information(msg.c_str()); }), "log");
    MODULE_ADD_LAMBDA(std::function<std::vector<chaiscript::Boxed_Value>(const std::string &, const std::string &, int)>(
        [](const std::string & str, const std::string & separators, int options)
        {
            auto chai_vect = std::vector<chaiscript::Boxed_Value>{};

            Poco::StringTokenizer st(str, separators, options);
            for (auto&& entry : st)
                chai_vect.emplace_back(chaiscript::var(entry));

            return chai_vect;
        }
    ), "stringTokenizer");
}

void ScriptDefs::modern_vars() const
{
    MODULE_ADD_GLOBAL_CONST_EASY(MICROSECOND_HOUR);
    MODULE_ADD_GLOBAL_CONST_EASY(MICROSECOND_DAY);
    MODULE_ADD_GLOBAL_CONST_EASY(DISCORD_WALLET_MASK);
    MODULE_ADD_GLOBAL_CONST_EASY(FIND_USER_UNKNOWN_USER);
    MODULE_ADD_GLOBAL_CONST_EASY(RPC_DATABASE_FILENAME);
    MODULE_ADD_GLOBAL_CONST_EASY(RPC_METHOD_GET_BALANCE);
    MODULE_ADD_GLOBAL_CONST_EASY(RPC_METHOD_GET_ADDRESS);
    MODULE_ADD_GLOBAL_CONST_EASY(RPC_METHOD_GET_BLK_HEIGHT);
    MODULE_ADD_GLOBAL_CONST_EASY(RPC_METHOD_TRANSFER);
    MODULE_ADD_GLOBAL_CONST_EASY(RPC_METHOD_SWEEP_ALL);
    MODULE_ADD_GLOBAL_CONST_EASY(RPC_METHOD_GET_TRANSFERS);
    MODULE_ADD_GLOBAL_CONST_EASY(RPC_METHOD_CREATE_WALLET);
    MODULE_ADD_GLOBAL_CONST_EASY(RPC_METHOD_OPEN_WALLET);
    MODULE_ADD_GLOBAL_CONST_EASY(RPC_METHOD_CLOSE_RPC);
    MODULE_ADD_GLOBAL_CONST_EASY(RPC_METHOD_STORE);
    MODULE_ADD_GLOBAL_CONST_EASY(RPC_METHOD_RESCAN_SPENT);
    MODULE_ADD_GLOBAL_CONST_EASY(RPC_METHOD_GET_TX_NOTE);
    MODULE_ADD_GLOBAL_CONST_EASY(RPC_METHOD_SET_TX_NOTE);
    MODULE_ADD_GLOBAL_CONST_EASY(VERSION_MAJOR);
    MODULE_ADD_GLOBAL_CONST_EASY(VERSION_MINOR);
}
#endif