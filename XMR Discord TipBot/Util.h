#pragma once
#include <string>

class Util
{
public:
	Util() = delete;
	~Util() = delete;

	static bool doesWalletExist(const std::string & name);
private:

};