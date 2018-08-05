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
#include "Script.h"
#include "Tipbot.h"
#ifndef NO_CHAISCRIPT
#include "chaiscript/chaiscript_stdlib.hpp"
#include "chaiscript/dispatchkit/bootstrap_stl.hpp"
#include "chaiscript/utility/utility.hpp"
#include "chaiscript/extras/math.hpp"
#include "Poco/File.h"
#include "Poco/Path.h"
#include <thread>
#include "Poco/Thread.h"
#include "RPCManager.h"
#include "Language.h"
#include "Config.h"
#include "AppBaseClass.h"
#include <cassert>
#endif

Script::Script(TIPBOT * DPTR) : DiscordPtr(DPTR)
{
#ifndef NO_CHAISCRIPT
    PLog = &Poco::Logger::get("Script Engine");
    preinit_engine();
#endif
}

#ifndef NO_CHAISCRIPT
Script::~Script()
{
    clearAll();
}

void Script::preinit_engine()
{
    mathscrdata = chaiscript::extras::math::bootstrap();
}

bool Script::add_script(const std::string& scriptPath)
{
    Poco::File f(scriptPath);
    if (f.exists())
    {
        PLog->information("Loading %s script!", scriptPath);
        if (is_script_loaded(scriptPath))
        {
            PLog->information("Duplicate found, reloading script!\n");

            // Delete script to reload.
            remove_script(scriptPath);
        }
        scripts.resize(scripts.size() + 1);
        class ScriptEngine & newEngine = scripts[scripts.size() - 1];
        newEngine.path = scriptPath;
        newEngine.engine = new chaiscript::ChaiScript;

        init_engine(newEngine);

        auto eval = [&]()
        {
            GlobalConfig.General.Threads++;
            PLog->information("Thread Started! Threads: %?i", GlobalConfig.General.Threads);
            try
            {
                newEngine.engine->eval_file(newEngine.path);

                // Get function data
                init_call_back_functions(newEngine);

                // Enter Main!
                newEngine.main();
            }
            catch (const chaiscript::exception::eval_error &ee)
            {
                script_exception(ee);
                remove_script(scriptPath);
            }
            newEngine.shutdown_complete = true;
            GlobalConfig.General.Threads--;
            PLog->information("Thread Stopped! Threads: %?i", GlobalConfig.General.Threads);
        };
        std::thread t(eval);
        t.detach();

        PLog->information("Script Loaded successfully!");
        return true; // Success!
    }
    else
    {
        PLog->information("Script failed to load -- File not found!");
    }
    return false; // Fail!
}

void Script::remove_script(const std::string& scriptPath)
{
    for (int i = 0; i < scripts.size(); i++)
    {
        if (scripts[i].path == scriptPath)
        {
            PLog->information("Removing Script %s", scriptPath);
            scripts[i].shutdown = true;
            while (!scripts[i].shutdown_complete) { Poco::Thread::sleep(1); }
            delete scripts[i].engine;
            scripts[i].engine = nullptr;
            scripts.erase(scripts.begin() + i);
            break;
        }
    }
}

bool Script::is_script_loaded(const std::string& scriptPath)
{
    for (int i = 0; i < scripts.size(); i++)
    {
        if (scripts[i].path == scriptPath)
            return true;
    }
    return false;
}

void Script::clearAll()
{
    for (int i = 0; i < scripts.size(); i++)
    {
        scripts[i].shutdown = true;
        while (!scripts[i].shutdown_complete) { Poco::Thread::sleep(1); }
        delete scripts[i].engine;
        scripts[i].engine = nullptr;
    }
    scripts.clear();
}

size_t Script::count() const
{
    return scripts.size();
}

bool Script::reinit_engine(class ScriptEngine& sEngine)
{
    Poco::File f(sEngine.path);
    if (f.exists())
    {
        PLog->information("Restarting script %s", sEngine.path);

        sEngine.shutdown = true;
        while (!sEngine.shutdown_complete) { Poco::Thread::sleep(1); }
        delete sEngine.engine;
        sEngine.engine = new chaiscript::ChaiScript;

        // Init Core
        init_engine(sEngine);

        auto eval = [&]()
        {
            GlobalConfig.General.Threads++;
            PLog->information("Thread Started! Threads: %?i", GlobalConfig.General.Threads);
            try
            {
                sEngine.engine->eval_file(sEngine.path);

                // Get function data
                init_call_back_functions(sEngine);

                // Enter Main!
                sEngine.main();
            }
            catch (const chaiscript::exception::eval_error &ee)
            {
                script_exception(ee);
            }
            sEngine.shutdown_complete = true;
            GlobalConfig.General.Threads--;
            PLog->information("Thread Stopped! Threads: %?i", GlobalConfig.General.Threads);
        };
        std::thread t(eval);
        t.detach();

        PLog->information("Script Reloaded successfully!");
        return true;
    }
    else
    {
        PLog->information("Could not restart script %s", sEngine.path);
    }

    // Load failed, destroy engine
    remove_script(sEngine.path);
    return false;
}

void Script::init_engine(class ScriptEngine& sEngine)
{
    try
    {
        // Math
        sEngine.engine->add(mathscrdata);

        // Tipbot
        sEngine.engine->add(tipbotdefs.getModule());

        // App Scripts
        auto & apps = DiscordPtr->getApps();
        for (auto & app : apps)
        {
            auto mod = app->getScriptModule();
            if (mod)
            {
                sEngine.engine->add(mod);
            }
        }

        ENGINE_ADD_GLOBAL(sEngine.engine, sEngine.shutdown, "script_shutdown");
        ENGINE_ADD_GLOBAL(sEngine.engine, DiscordPtr, "DiscordPtr");
        ENGINE_ADD_GLOBAL(sEngine.engine, *RPCMan, "RPCMan");
        ENGINE_ADD_GLOBAL_EASY(sEngine.engine, GlobalLanguage);
        ENGINE_ADD_GLOBAL_EASY(sEngine.engine, GlobalConfig);
    }
    catch (const chaiscript::exception::eval_error &ee)
    {
        PLog->information(ee.pretty_print());
    }
}

void Script::script_exception(const chaiscript::exception::eval_error & ee)
{
    PLog->information(ee.pretty_print());
}

const std::vector<class ScriptEngine> & Script::getScripts()
{
    return scripts;
}

void Script::init_call_back_functions(class ScriptEngine& sEngine)
{
    auto funcs = sEngine.engine->get_state().engine_state.m_functions;

    for (const auto & func : funcs)
    {
        if (func.first == "main")
        {
            sEngine.main = sEngine.engine->eval<std::function<void()>>("main");
        }
        else if (func.first == "onMessage")
        {
            sEngine.OnMessage = sEngine.engine->eval<std::function<void(const UserMessage*)>>("onMessage");
        }
    }

    // Core Functions.
    if (!sEngine.main)
    {
        PLog->information("Could not find/load main! main is required!");
        remove_script(sEngine.path);
    }
}

void Script::call_back(ecallback type, const std::vector<const void*>& data)
{
    for (auto & scr : scripts)
    {
        try
        {
            switch (type)
            {
            case ecallback::OnMessage:
                if (scr.OnMessage)
                {
                    scr.OnMessage(static_cast<const UserMessage*>(data[0]));
                }
                break;
            default:
                assert(0);
            }
        }
        catch (const chaiscript::exception::eval_error &ee)
        {
            script_exception(ee);
            remove_script(scr.path);

            // An error occred and we need to exit the loop.
            break;
        }
    }
}
#endif