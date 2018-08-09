# Monero-TipBot
Discord Tipbot built in C++ for Monero/IntenseCoin

Discord: https://discord.gg/rpTU2T

## Compile

### Windows
* Visual Studio 2017: File -> Open -> CMake -> Click CMakeLists.txt

### Linux
* cmake .
* make 

## Applications

Tipbot has 4 major applications that are disabled by default. The first applicaiton is of cource Tipbot which processes user wallet commands. The second application is the faucet which allows users to take a percentage of the faucet wallet using the !take command. The third application is a lottery application that runs weekly starting on Saturday and ending on Friday 6 PM UTC. Around 9 PM UTC on Friday a winner will be drawn who will be sent the jackpot pool or a percentage of the jackpot pool if you want to donate some back to the Faucet. Lastly, Projects is the crowd funding applications which allows people to pool money together for projects and award them to a user once a project is completed.

### Tip

#### Public Commands
* !about 
* !tipbot 
* !listlanguage 
* !selectlanguage 
* !myaddress  -- Direct Message Only
* !blockheight 
* !balance 
* !history  -- Direct Message Only
* !withdraw [amount] [address] -- Direct Message Only
* !withdrawall [address] -- Direct Message Only
* !give [amount] [@User1 @User2...] -- Public Channel Only
* !giveall [@User] -- Public Channel Only
* !tip [amount] [@User1 @User2...] -- Public Channel Only
* !tipall [@User] -- Public Channel Only
* !restartwallet 
* !uptime 

#### Admin Commands
* !togglewithdraw  -- Direct Message Only -- ADMIN ONLY
* !togglegive  -- Direct Message Only -- ADMIN ONLY
* !rescanallwallets  -- Direct Message Only -- ADMIN ONLY
* !totalbalance  -- Direct Message Only -- ADMIN ONLY
* !savewallets  -- Direct Message Only -- ADMIN ONLY
* !restartfaucet  -- Direct Message Only -- ADMIN ONLY
* !softrestart  -- Direct Message Only -- ADMIN ONLY
* !shutdown  -- Direct Message Only -- ADMIN ONLY
* !rpcstatus  -- Direct Message Only -- ADMIN ONLY
* !whois [DiscordID] -- Direct Message Only -- ADMIN ONLY
* !performance  -- Direct Message Only -- ADMIN ONLY
* !executing  -- Direct Message Only -- ADMIN ONLY
* !toggletipbot  -- Direct Message Only -- ADMIN ONLY

### Faucet

#### Public Commands
* !faucet
* !take

#### Admin Commands
* !status  -- Direct Message Only -- ADMIN ONLY
* !togglefaucet  -- Direct Message Only -- ADMIN ONLY
* !award [@User] -- ADMIN ONLY

### Lottery
#### Public Commands
* !lottery
* !jackpot
* !gameinfo
* !mytickets
* !buytickets [amount]
* !waslotterywon 

#### Admin Commands:
* !togglelottery  -- Direct Message Only -- ADMIN ONLY
* !lastwinner  -- Direct Message Only -- ADMIN ONLY

### Chat Rewards:

### Admin Commands:
* !chatrewards  -- Direct Message Only -- ADMIN ONLY
* !disallowid [id] -- Direct Message Only -- ADMIN ONLY
* !allowid [id] -- Direct Message Only -- ADMIN ONLY
* !setchannel [id] -- Direct Message Only -- ADMIN ONLY
* !paymentqueuesize  -- Direct Message Only -- ADMIN ONLY
* !roundusersize  -- Direct Message Only -- ADMIN ONLY
* !togglechatrewards  -- Direct Message Only -- ADMIN ONLY

### Projects
#### Public Commands
* !projects 
* !fundproject [amount] "[project]"
* !listprojects 
* !viewstatus "[project]"
* !projectaddress "[project]"

#### Admin Commands:
* !create "[project]" "[description]" [goal] -- Direct Message Only -- ADMIN ONLY
* !delete "[project]" -- Direct Message Only -- ADMIN ONLY
* !grantuser "[project]" [user] -- ADMIN ONLY
* !toggleproject "[project]" -- Direct Message Only -- ADMIN ONLY
* !toggleprojects "[project]" -- Direct Message Only -- ADMIN ONLY

## Requirements
* IntenseCon/Monero Daemon
* IntenseCoin/Monero RPC
* Discord Token 
* TCP Port 11000 to 11100 free to bind (can be redefined in config)

## Dependencies (included)

### Windows/Linux
* Poco C++ - https://github.com/pocoproject
* Sleepy Discord - https://github.com/yourWaifu/sleepy-discord

### Linux Packages
* libcurl4-openssl-dev
* libssl-dev

## RPC Setup
You need to download the RPC for the coin you want to setup the bot with, for example, IntenseCoin (https://github.com/valiant1x/intensecoin/releases). Unzip it to whereever you built the TIPBOT.

## Deamon setup
Next you'll need to open the daemon for the coin, for example, `intensecoind` for IntenseCoin.

The file structure should look like this:
* intensecoind/monerocoind
* intense-wallet-rpc/monero-wallet-rpc
* TIPBOT <- This programs executable. 
* Wallets/ <-- this is a directory. This will be created after running TIPBOT.

## Discord Token Setup
Navigate to "My Apps" on Discord (https://discordapp.com/developers/applications/me). 
Click new app, give it a name and description. 
Click create a bot user.
Copy the token generated on that page and open TIPBOT and enter it when prompted.
To add a bot user to your account you'll need to get the client id for the bot which is also on that page. 
Then navigate to this url (https://discordapp.com/oauth2/authorize?client_id=CLIENTID&scope=bot) and replace CLIENTID in the url with the client url on the page.

The bot should now be ready to run. Launch it, enter the token, and type `!balance` and if it returns a balance of 0 then the communication between the bot and the RPC is working correctly. 

Author: Brandan Tyler Lasley
* BTC: 1KsX66J98WMgtSbFA5UZhVDn1iuhN5B6Hm
* ITNS: iz5ZrkSjiYiCMMzPKY8JANbHuyChEHh8aEVHNCcRa2nFaSKPqKwGCGuUMUMNWRyTNKewpk9vHFTVsHu32X3P8QJD21mfWJogf
* XMR: 44DudyMoSZ5as1Q9MTV6ydh4BYT6BMCvxNZ8HAgeZo9SatDVixVjZzvRiq9fiTneykievrWjrUvsy2dKciwwoUv15B9MzWS
* MSR: 5h9GZz5bbvUK5TPb1KB8J7FnbQHyEd1z93scwhu3WZ9m3YJwCAUVyz3FoKh4JiTTWPKcGmJkxBWS2YkmzJoXTimqTbCKFKm
