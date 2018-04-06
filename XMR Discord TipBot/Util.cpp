#include "Util.h"

#include "Poco/File.h"
#include "Poco/Process.h"
#include <assert.h>

bool Util::doesWalletExist(const std::string & name)
{
	return Poco::File(name).exists();
}
