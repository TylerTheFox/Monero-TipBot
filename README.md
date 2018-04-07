# IntenseCoin-TipBot
Discord Tipbot built in C++ for IntenseCoin/Monero

## Compile
It should compile without issue in Visual Studio 2017, however, the externals are only compiled for x86 mode. Should run on Windows, Mac, Linux but the external libraries will need to be recompiled for that platform.

## Commands
* !help
* !balance
* !myaddress
* !history
* !withdraw [amount] [address]
* !withdrawall [amount]
* !give [amount] [@User1 @User2...]
* !giveall [@User]

## Requirements
* IntenseCon Daemon
* IntenseCoin RPC - Configuration can be found in RPC.h (IP/Port)
* Discord Token 

## Dependencies (included)
* Poco C++ - https://github.com/pocoproject
* Sleepy Discord - https://github.com/yourWaifu/sleepy-discord

## IntenseCoin Deamon setup
Just open `intensecoind.exe` by double clicking on it.

## RPC Setup
First you need to download the RPC from IntenseCoin if you haven't already (https://github.com/valiant1x/intensecoin/releases). 
Unzip the folder and run 
`intense-wallet-rpc.exe` with the arguments `--wallet-dir ./Wallets/  --rpc-bind-port 8333 --daemon-address 127.0.0.1:48782 --disable-rpc-login --trusted-daemon`

## Application Setup
The program working directory needs to be set to the folder RPC is in.

## Discord Token Setup
Navigate to "My Apps" on Discord (https://discordapp.com/developers/applications/me). 
Click new app, give it a name and description. 
Click create a bot user.
Copy the token generated on that page and paste into main.cpp where it says `DISCORD TOKEN`.
To add a bot user to your account you'll need to get the client id for the bot which is also on that page. 
Then navigate to this url (https://discordapp.com/oauth2/authorize?client_id=CLIENTID&scope=bot) and replace CLIENTID in the url with the client url on the page.

The bot should now be ready to run. Launch it and type `!balance` and if it returns a balance of 0 then the communication between the bot and the RPC is working correctly. 
