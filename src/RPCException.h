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
#include <string>
#include <exception>

class AppGeneralException : public std::exception
{
public:
    virtual ~AppGeneralException() = default;
    virtual const std::string & getGeneralError() = 0;
    virtual const char* what() = 0;
};

class RPCConnectionError : public AppGeneralException
{
public:
    RPCConnectionError(std::string general_error);
    const std::string & getGeneralError() override;
    const char* what() override;
private:
    std::string genErr;
};

class RPCGeneralError : public AppGeneralException
{
public:
    RPCGeneralError(const std::string& code, std::string general_error);
    const std::string & getGeneralError() override;
    const char* what() override;
private:
    std::string code;
    std::string genErr;
};