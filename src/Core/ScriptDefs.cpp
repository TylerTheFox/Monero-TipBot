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
#include "Tipbot.h"
#include "ScriptDefs.h"
#include "Script.h"
#include "Poco/Thread.h"
#include "RPCManager.h"
#include "RPC.h"
#include "Account.h"
#include "Config.h"
#include "Util.h"

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
    MODULE_ADD_TYPE_FULL_EASY(RPCManager);
    MODULE_ADD_TYPE_BASIC_EASY(TIPBOT);
    MODULE_ADD_TYPE_FULL_EASY(RPC);             // Done
    MODULE_ADD_TYPE_FULL_EASY(BalanceRet);      // Done
    MODULE_ADD_TYPE_FULL_EASY(TransferRet);     // Done
    MODULE_ADD_TYPE_FULL_EASY(TransferItem);    // Done
    MODULE_ADD_TYPE_FULL_EASY(TransferList);    // Done
    MODULE_ADD_TYPE_FULL_EASY(Account);
    MODULE_ADD_TYPE_FULL_EASY(AboutConfig);
    MODULE_ADD_TYPE_FULL_EASY(GeneralConfig);
    MODULE_ADD_TYPE_FULL_EASY(RPCConfig);
    MODULE_ADD_TYPE_FULL_EASY(RPCManagerConfig);
    MODULE_ADD_TYPE_FULL_EASY(FacuetConfig);
    MODULE_ADD_TYPE_FULL_EASY(LotteryConfig);
    MODULE_ADD_TYPE_FULL_EASY(ChatRewardsConfig);
    MODULE_ADD_TYPE_FULL_EASY(AppConfig);
    MODULE_ADD_TYPE_FULL_EASY(UserMessage);
    MODULE_ADD_TYPE_FULL_EASY(Command);
    MODULE_ADD_TYPE_FULL_EASY(Snowflake);
    MODULE_ADD_TYPE_FULL_EASY(TopTakerStruct);
    MODULE_ADD_TYPE_FULL_EASY(DiscordUser);
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
            { chaiscript::fun(&RPC::tranfer), "tranfer" },
            { chaiscript::fun(&RPC::sweepAll), "sweepAll" },
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
}
void ScriptDefs::class_functions() const
{

}

void ScriptDefs::core_functions()
{
    MODULE_ADD_LAMBDA(std::function<void(const std::string &)>([&](const std::string & msg) { PLog->information(msg.c_str()); }), "log");
    MODULE_ADD_LAMBDA(Poco::Thread::sleep, "sleep");
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