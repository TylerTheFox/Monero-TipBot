# IntenseCoin-TipBot
Discord Tipbot built in C++ for IntenseCoin/Monero

## Compile

### Windows
* Visual Studio 2017: File -> Open -> CMake -> Click CMakeLists.txt

### Linux
* cmake .
* make 

## Commands
* !help
* !balance
* !myaddress
* !history
* !withdraw [amount] [address]
* !withdrawall [address]
* !give [amount] [@User1 @User2...]
* !giveall [@User]

## Requirements
* IntenseCon Daemon
* IntenseCoin RPC - Configuration can be found in RPC.h (IP/Port)
* Discord Token 

## Dependencies (included)

### Windows/Linux
* Poco C++ - https://github.com/pocoproject
* Sleepy Discord - https://github.com/yourWaifu/sleepy-discord

### Linux Packages
* libcurl4-openssl-dev
* libssl-dev

## RPC Setup
First you need to download the RPC from IntenseCoin if you haven't already (https://github.com/valiant1x/intensecoin/releases). 
Unzip the folder and run 
`intense-wallet-rpc` with the arguments `--wallet-dir ./Wallets/  --rpc-bind-port 8333 --daemon-address 127.0.0.1:48782 --disable-rpc-login --trusted-daemon`

Note: `--wallet-dir ./Wallets/` is required and to change the dir you must change RPC.h

It should complain about the deamon not running. Thats normal see the next step.

## IntenseCoin Deamon setup
Just open `intensecoind` normally.

## Application Setup
The program needs to be in the folder with RPC or have the working directory set to the folder RPC is in.

The file structure should look like this:
* intense-wallet-rpc
* TIPBOT
* Wallets/ <-- this is a directory. 

## Discord Token Setup
Navigate to "My Apps" on Discord (https://discordapp.com/developers/applications/me). 
Click new app, give it a name and description. 
Click create a bot user.
Copy the token generated on that page and open TIPBOT and when prompted enter the token.
To add a bot user to your account you'll need to get the client id for the bot which is also on that page. 
Then navigate to this url (https://discordapp.com/oauth2/authorize?client_id=CLIENTID&scope=bot) and replace CLIENTID in the url with the client url on the page.

The bot should now be ready to run. Launch it and type `!balance` and if it returns a balance of 0 then the communication between the bot and the RPC is working correctly. 

Author: Brandan Tyler Lasley
* BTC: 1KsX66J98WMgtSbFA5UZhVDn1iuhN5B6Hm
* ITNS: iz5ZrkSjiYiCMMzPKY8JANbHuyChEHh8aEVHNCcRa2nFaSKPqKwGCGuUMUMNWRyTNKewpk9vHFTVsHu32X3P8QJD21mfWJogf
* XMR: 44DudyMoSZ5as1Q9MTV6ydh4BYT6BMCvxNZ8HAgeZo9SatDVixVjZzvRiq9fiTneykievrWjrUvsy2dKciwwoUv15B9MzWS
