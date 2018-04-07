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
#include "RPCException.h"
#include <utility>

RPCConnectionError::RPCConnectionError(std::string general_error) : genErr(std::move(general_error))
{
}

const std::string & RPCConnectionError::getGeneralError()
{
	return genErr;
}

const char * RPCConnectionError::what()
{
	return "Connection to the RPC Failed! Ensure RPC is running and the port is correct. Contact @Admins for help.";
}

RPCGeneralError::RPCGeneralError(const std::string& code, std::string general_error) : genErr(std::move(general_error))
{
}

const std::string & RPCGeneralError::getGeneralError()
{
	return genErr;
}

const char * RPCGeneralError::what()
{
	return "RPC error, check logs for more infomation. If this continues contact @Admins";
}