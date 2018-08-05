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
#include <unordered_map>
#include <string>
#include <vector>
#include "cereal/types/unordered_map.hpp"

struct Lang
{
    std::string                                     LangName;
    std::unordered_map<std::string, std::string>    LangMap;

    template <class Archive>
    void serialize(Archive & ar)
    {
        ar(
            CEREAL_NVP(LangName),
            CEREAL_NVP(LangMap)
        );
    }
};

class LanguageSelect
{
public:
    LanguageSelect();

    const std::string &         getString(uint8_t language, const std::string & label);
    void                        getLanguages(std::vector<std::string> & vect);
    size_t                      size();

    template <class Archive>
    void serialize(Archive & ar)
    {
        ar(
            CEREAL_NVP(LocalLanguage)
        );
    }
private:
    std::vector< struct Lang > LocalLanguage;
};

extern class LanguageSelect GlobalLanguage;
#define GETSTR(l, n) (GlobalLanguage.getString(l, n))