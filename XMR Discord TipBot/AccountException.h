#pragma once
#include "RPCException.h"
#include <string>

class InsufficientBalance : public AppGeneralException
{
public:
	InsufficientBalance(const std::string & general_error);
	const std::string & getGeneralError();
	const char* what();
private:
	std::string code;
	std::string genErr;
};

class ZeroTransferAmount : public AppGeneralException
{
public:
	ZeroTransferAmount(const std::string & general_error);
	const std::string & getGeneralError();
	const char* what();
private:
	std::string code;
	std::string genErr;
};

class GeneralAccountError : public AppGeneralException
{
public:
	GeneralAccountError(const std::string & general_error);
	const std::string & getGeneralError();
	const char* what();
private:
	std::string code;
	std::string genErr;
};