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
#include "Tipbot.h"

class Account;

class AppBaseClass
{
public:
    virtual ~AppBaseClass() = default;
    AppBaseClass() = default;

    virtual void            setAccount(Account *) = 0;
    virtual void            save() = 0;
    virtual void            load() = 0;

    virtual iterator        begin() = 0;
    virtual const_iterator  begin() const = 0;
    virtual const_iterator  cbegin() const = 0;

    virtual iterator        end() = 0;
    virtual const_iterator  end() const = 0;
    virtual const_iterator  cend() const = 0;

    virtual void            run(const UserMessage & message) { };
};
