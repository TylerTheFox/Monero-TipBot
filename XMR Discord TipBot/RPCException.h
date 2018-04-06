#pragma once
#include <exception>
#include <string>

class RPCConnectionFailed : public std::exception
{
public:
	RPCConnectionFailed(const std::string& general_error);
	const std::string & getGeneralError();
	const char* what();
private:
	std::string genErr;
};

class RPCGeneralError : public std::exception
{
public:
	RPCGeneralError(const std::string& code, const std::string & general_error);
	const std::string & getGeneralError();
	const char* what();
private:
	std::string code;
	std::string genErr;
};