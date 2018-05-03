#pragma once
#include "Discord.h"

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
    virtual const_iterator  cbegin() const =0;

    virtual iterator        end() = 0;
    virtual const_iterator  end() const= 0;
    virtual const_iterator  cend() const = 0;
};
