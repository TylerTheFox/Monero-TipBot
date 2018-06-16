#include "CLI.h"

CLI::CLI(TIPBOT * DiscordPtr)
{

}

CLI::~CLI()
{

}

void CLI::save()
{
}

void CLI::load()
{
}

void CLI::setAccount(Account* acc)
{

}

iterator CLI::begin()
{
    return Commands.begin();
}

const_iterator CLI::begin() const
{
    return Commands.begin();
}

const_iterator CLI::cbegin() const
{
    return Commands.cbegin();
}

iterator CLI::end()
{
    return Commands.end();
}

const_iterator CLI::end() const
{
    return Commands.end();
}

const_iterator CLI::cend() const
{
    return Commands.cend();
}