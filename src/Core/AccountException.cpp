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
#include "AccountException.h"
#include <utility>

InsufficientBalance::InsufficientBalance(std::string general_error) : genErr(std::move(general_error))
{
}

const std::string & InsufficientBalance::getGeneralError()
{
    return genErr;
}

const char * InsufficientBalance::what()
{
    return "Insufficient Balance";
}

ZeroTransferAmount::ZeroTransferAmount(std::string general_error) : genErr(std::move(general_error))
{
}

const std::string & ZeroTransferAmount::getGeneralError()
{
    return genErr;
}

const char * ZeroTransferAmount::what()
{
    return "Transfer Error";
}

GeneralAccountError::GeneralAccountError(std::string general_error) : genErr(std::move(general_error))
{
}

const std::string & GeneralAccountError::getGeneralError()
{
    return genErr;
}

const char * GeneralAccountError::what()
{
    return "General Application Error";
}
