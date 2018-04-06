#pragma once
#include <exception>
#include <string>
class RPCExceptiion
{
public:
	virtual const std::string & getGeneralError() = 0;
	virtual const char* what() = 0;
};

class RPCConnectionFailed : public RPCExceptiion
{
public:
	RPCConnectionFailed(const std::string& general_error);
	const std::string & getGeneralError();
	virtual const char* what();
private:
	std::string genErr;
};

class RPCGeneralError : public RPCExceptiion
{
public:
	RPCGeneralError(const std::string& code, const std::string & general_error);
	const std::string & getGeneralError();
	const char* what();
private:
	std::string code;
	std::string genErr;
};