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
#pragma once
#ifndef NO_CHAISCRIPT
#include "chaiscript/chaiscript.hpp"
#include "Poco/Logger.h"

class ScriptDefs
{
public:
    ScriptDefs();
    chaiscript::ModulePtr & getModule();
private:
    chaiscript::ModulePtr m;
    Poco::Logger*         PLog;

    void core_datatypes() const;
    void core_datatypes_impli() const;
    void class_functions() const;
    void core_functions();
    void modern_vars() const;
};
#endif