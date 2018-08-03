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
#include "chaiscript/chaiscript_stdlib.hpp"
#include "chaiscript/dispatchkit/bootstrap_stl.hpp"
#include "chaiscript/utility/utility.hpp"
#include "chaiscript/extras/math.hpp"
#include "Poco/File.h"
#include "Poco/Path.h"

Script::Script()
{
    preinit_engine();
}

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
        if (is_script_loaded(scriptPath))
        {
            //::trace("Duplicate found, reloading script!\n");

            // Delete script to reload.
            remove_script(scriptPath);
        }
        scripts.resize(scripts.size() + 1);
        class ScriptEngine & newEngine = scripts[scripts.size() - 1];
        newEngine.path = scriptPath;
        newEngine.engine = new chaiscript::ChaiScript;

        init_engine(newEngine);

        try
        {
            newEngine.engine->eval_file(scriptPath);

            // Get function data
            init_call_back_functions(newEngine);

            //::trace("Script Loaded successfully!");
            return true; // Success!
        }
        catch (const chaiscript::exception::eval_error &ee)
        {
            script_exception(ee);
            remove_script(scriptPath);
        }
    }
    else
    {
        //::trace("Script failed to load -- File not found!");
    }
    return false; // Fail!
}

void Script::remove_script(const std::string& scriptPath)
{
    for (int i = 0; i < scripts.size(); i++)
    {
        if (scripts[i].path == scriptPath)
        {
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
        delete sEngine.engine;
        sEngine.engine = new chaiscript::ChaiScript;

        // Init Core
        init_engine(sEngine);

        try
        {
            sEngine.engine->eval_file(sEngine.path);

            // Get function data
            init_call_back_functions(sEngine);

            //::trace("Script Reloaded successfully!");
            return true;
        }
        catch (const chaiscript::exception::eval_error &ee)
        {
            script_exception(ee);
        }
    }
    else
    {
        // Load Fail
    }

    // Load failed, destroy engine
    remove_script(sEngine.path);
    return false;
}

void Script::init_engine(class ScriptEngine& sEngine)
{
#if !CM_NO_SCRIP3_OR_SAVE
    try
    {
        // Math
        sEngine.engine->add(mathscrdata);

        // Tipbot
        sEngine.engine->add(tipbotdefs.getModule());
    }
    catch (const chaiscript::exception::eval_error &ee)
    {
        //::trace(ee.pretty_print().c_str());
    }
#endif
}

void Script::init_call_back_functions(class ScriptEngine& sEngine)
{

}

void Script::script_exception(const chaiscript::exception::eval_error & ee)
{
    std::stringstream poperr;
    Poco::Path scrpth(ee.filename);
    poperr << "Script Error: " << ee.reason << " in '" << scrpth.getFileName() << "' at (" << ee.start_position.line << ", " << ee.start_position.column << ")";
    const std::string & poperrstr = poperr.str();
    //::trace(ee.pretty_print().c_str());
}