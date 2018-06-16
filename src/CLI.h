#include "Discord.h"
#include "Account.h"
#include "AppBaseClass.h"

class CLI : public AppBaseClass
{
public:
    CLI(TIPBOT * DiscordPtr);
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
private:
    std::vector<struct Command>         Commands;
};