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