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
#include "RPCException.h"
#include <string>

class InsufficientBalance : public AppGeneralException
{
public:
	InsufficientBalance(std::string general_error);
	const std::string & getGeneralError() override;
	const char* what() override;
private:
	std::string code;
	std::string genErr;
};

class ZeroTransferAmount : public AppGeneralException
{
public:
	ZeroTransferAmount(std::string general_error);
	const std::string & getGeneralError() override;
	const char* what() override;
private:
	std::string code;
	std::string genErr;
};

class GeneralAccountError : public AppGeneralException
{
public:
	GeneralAccountError(std::string general_error);
	const std::string & getGeneralError() override;
	const char* what() override;
private:
	std::string code;
	std::string genErr;
};