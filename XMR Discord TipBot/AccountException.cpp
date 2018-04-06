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
