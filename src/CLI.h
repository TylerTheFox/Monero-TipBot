#include "Discord.h"
#include "Account.h"
#include "AppBaseClass.h"
#include "Poco/Logger.h"

class CLI : public AppBaseClass
{
public:
    CLI(TIPBOT * dptr);
    virtual ~CLI();

    void                                save();
    void                                load();
    void                                setAccount(Account *);
    iterator                            begin();
    const_iterator                      begin() const;
    const_iterator                      cbegin() const;

    iterator                            end();
    const_iterator                      end() const;
    const_iterator                      cend() const;

    void                                cli_main();

    UserMessage                         generateUsrMsg(std::string msg);
private:
    TIPBOT*                             DiscordPtr;
    Poco::Logger*                       PLog;
    std::vector<struct Command>         Commands;
};