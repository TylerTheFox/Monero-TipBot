#include "AccountException.h"

InsufficientBalance::InsufficientBalance(const std::string & general_error) : genErr(general_error)
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

ZeroTransferAmount::ZeroTransferAmount(const std::string & general_error) : genErr(general_error)
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

GeneralAccountError::GeneralAccountError(const std::string & general_error) : genErr(general_error)
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
