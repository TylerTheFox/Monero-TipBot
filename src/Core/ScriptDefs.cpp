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
#include "ScriptDefs.h"
#include "Script.h"

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


}

void ScriptDefs::core_datatypes_impli() const
{

}

void ScriptDefs::class_functions() const
{
    MODULE_ADD_LAMBDA(std::function<void(const std::string &)>([&](const std::string & msg) { PLog->information(msg.c_str()); }), "log");
}

void ScriptDefs::core_functions()
{

}

void ScriptDefs::modern_vars() const
{

}