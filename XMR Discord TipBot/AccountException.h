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