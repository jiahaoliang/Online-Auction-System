a. Full Name: Jiahao Liang

b. Student ID: 4461559794

c. Works done:
Finished all 3 phases according to the project requirement. Used fork() to create two bidders and two sellers in one bidder.c and seller.c. Provided Makefile to compile my program automatically.

d. Introductions to code files:

auctionserver.c: acts as the server in all phases.

bidder.c: acts as bidder. Use fork() create two processes indicating bidder1 and bidder2 respectively.

seller.c: acts as seller. Use fork() create two processes indicating seller1 and seller2 respectively.

EE450.h: contains all the constants and structures definition shared by all auctionserver.c, bidder.c and seller.c.

EE450.c: contains all the functions shared by all auctionserver.c, bidder.c and seller.c.

singlyLinkedList.h: implements a singly linked list used by all auctionserver.c, bidder.c and seller.c. Contains all the constants and structures definition. 

singlyLinkedList.c: implements a singly linked list used by all auctionserver.c, bidder.c and seller.c. Contains all the list operation functions. 

Makefile: a script to compile my codes automatically.

README.txt: the file you are reading now.

e. Running the program:
(1) Unzip the tar.gz file into one folder.
(2) use “cd” to change directory to the target folder
(3) Run “make clean” to make sure any files unnecessary is deleted. 
(4) Simply run “make”. All the codes will be compiled automatically, then we will have 3 executable files “auctionserver”, “bidder” and “seller” 
(5) First, run “auctionserver”; Second, run “bidder”; Third, run “seller”. Try to minimize the intervals between runs as possible, like within 5 seconds.

f. Format of exchanging messages:
All the formats are in accordance with requirements in the project document:

/**************************************************************************
 *The auctionserver prints the following:
 **************************************************************************/
Phase 1: Auction server has TCP port number 1894 and IP address 68.181.201.3
server: waiting for connections...
Phase 1: Authentication request. User1: Username:Mary Password:123456 Bank Account:451912345 User IP Addr:68.181.201.3 Authorized:Accepted#
Phase 1: Authentication request. User2: Username:James Password:pass123 Bank Account:451965432 User IP Addr:68.181.201.3 Authorized:Accepted#
Phase 1: Authentication request. User3: Username:Taylor Password:abc654 Bank Account:451934578 User IP Addr:68.181.201.3 Authorized:Accepted#
Phase 1: Auction Server IP Address:68.181.201.3 PreAuction Port Number:1994 sent to the <Seller1>
Phase 1: Authentication request. User4: Username:Julia Password:654321 Bank Account:451900001 User IP Addr:68.181.201.3 Authorized:Accepted#
Phase 1: Auction Server IP Address:68.181.201.3 PreAuction Port Number:1994 sent to the <Seller2>
End of Phase 1 for Auction Server

Phase 2: Auction Server IP Address:68.181.201.3 PreAuction TCP Port Number:1994 .
Phase 2: <Seller1> send item lists.
Taylor
Skirt1 50
Shirt1 100
Watch1 120
Phase 2: <Seller2> send item lists.
Julia
Skirt1 80
Shirt2 100
Watch1 100
Watch2 150
End of Phase 2 for Auction Server

Phase 3: Auction Server IP Address:68.181.201.3 Auction UDP Port Number:41430 .
Phase 3: (Item list displayed here)
Taylor Skirt1 50
Taylor Shirt1 100
Taylor Watch1 120
Julia Skirt1 80
Julia Shirt2 100
Julia Watch1 100
Julia Watch2 150
Phase 3: Auction Server IP Address:68.181.201.3 Auction UDP Port Number:41431 .
Phase 3: (Item list displayed here)
Taylor Skirt1 50
Taylor Shirt1 100
Taylor Watch1 120
Julia Skirt1 80
Julia Shirt2 100
Julia Watch1 100
Julia Watch2 150
Phase 3: Auction Server received a bidding from <Bidder1>
Phase 3: (Bidding information displayed here)
Taylor Skirt1 100
Taylor Watch1 80
Julia Watch1 150
Phase 3: Auction Server received a bidding from <Bidder2>
Phase 3: (Bidding information displayed here)
Taylor Skirt1 100
Taylor Watch1 200
Julia Skirt1 100
Julia Shirt2 110
Phase 3: Item: Taylor Skirt1 was sold at price 100 .
Phase 3: Item: Taylor Watch1 was sold at price 200 .
Phase 3: Item: Julia Skirt1 was sold at price 100 .
Phase 3: Item: Julia Shirt2 was sold at price 110 .
Phase 3: Item: Julia Watch1 was sold at price 150 .
End of Phase 3 for Auction Server.

/**************************************************************************
 *The bidder prints the following:
 **************************************************************************/
Phase 1: <Bidder1> has TCP port 48080 and IP address: 68.181.201.3
Phase 1: Login request. User:Mary password:123456 Bank account:451912345
Phase 1: Login request reply: Accepted# .

Phase 1: <Bidder2> has TCP port 48081 and IP address: 68.181.201.3
Phase 1: Login request. User:James password:pass123 Bank account:451965432
Phase 1: Login request reply: Accepted# .

Phase 3: <Bidder1> has UDP port 3894 and IP address: 68.181.201.3
Phase 3: <Bidder1> (Item list displayed here)
Taylor Skirt1 50
Taylor Shirt1 100
Taylor Watch1 120
Julia Skirt1 80
Julia Shirt2 100
Julia Watch1 100
Julia Watch2 150
Phase 3: <Bidder1> (Bidding information displayed here)
Taylor Skirt1 100
Taylor Watch1 80
Julia Watch1 150
Phase 3: <Bidder2> has UDP port 3994 and IP address: 68.181.201.3
Phase 3: <Bidder2> (Item list displayed here)
Taylor Skirt1 50
Taylor Shirt1 100
Taylor Watch1 120
Julia Skirt1 80
Julia Shirt2 100
Julia Watch1 100
Julia Watch2 150
Phase 3: <Bidder2> (Bidding information displayed here)
Taylor Skirt1 100
Taylor Watch1 200
Julia Skirt1 100
Julia Shirt2 110
Phase 3: <Bidder1>
Phase 3: Item: Julia Watch1 was sold at price 150 .
End of Phase 3 for <Bidder1>.

Phase 3: <Bidder2>
Phase 3: Item: Taylor Skirt1 was sold at price 100 .
Phase 3: Item: Taylor Watch1 was sold at price 200 .
Phase 3: Item: Julia Skirt1 was sold at price 100 .
Phase 3: Item: Julia Shirt2 was sold at price 110 .
End of Phase 3 for <Bidder2>.

/**************************************************************************
 *The seller prints the following:
 **************************************************************************/
Phase 1: <Seller1> has TCP port 48088 and IP address: 68.181.201.3
Phase 1: Login request. User:Taylor password:abc654 Bank account:451934578
Phase 1: Login request reply: Accepted# .
Phase 1: Auction Server has IP Address:68.181.201.3 and PreAuction TCP Port Number:1994
End of Phase 1 for <Seller1>.

Phase 1: <Seller2> has TCP port 48091 and IP address: 68.181.201.3
Phase 1: Login request. User:Julia password:654321 Bank account:451900001
Phase 1: Login request reply: Accepted# .
Phase 1: Auction Server has IP Address:68.181.201.3 and PreAuction TCP Port Number:1994
End of Phase 1 for <Seller2>.

Phase 2: Auction Server has IP Address:68.181.201.3 PreAuction Port Number:1994
Phase 2: <Seller1> send item lists.
Taylor
Skirt1 50
Shirt1 100
Watch1 120
End of Phase 2 for <Seller1>.

Phase 2: Auction Server has IP Address:68.181.201.3 PreAuction Port Number:1994
Phase 2: <Seller2> send item lists.
Julia
Skirt1 80
Shirt2 100
Watch1 100
Watch2 150
End of Phase 2 for <Seller2>.

Phase 3: <Seller1>
Phase 3: Item: Taylor Skirt1 was sold at price 100 .
Phase 3: Item: Taylor Watch1 was sold at price 200 .
End of Phase 3 for <Seller1>.

Phase 3: <Seller2>
Phase 3: Item: Julia Skirt1 was sold at price 100 .
Phase 3: Item: Julia Shirt2 was sold at price 110 .
Phase 3: Item: Julia Watch1 was sold at price 150 .
End of Phase 3 for <Seller2>.

/**************************************************************************
 *Note: All dynamic ports might be different when executing.
 **************************************************************************/

g. Idiosyncracy of the project:
(1) The host name "nunki.usc.edu" is hardcoded.
(2) The Static port numbers is hardcoded in “EE450.h”.
(3) Filenames and format of input txt files should be EXACTLY the same with project description and example test files provided.
(4) My codes can only support exactly 4 users, namely, 2 bidders and 2 sellers. 
(5) If there is any user (bidder or seller) rejected at Phase 1. The auctionserver will shutdown after finish Phase 1. All the following connection to server will fail. You should press “ctrl+c” to exit processes still running manually.
(6) Press “ctrl+c” anytime to quit any program immediately. 
(7) Input files for “auctionserver”: “Registration.txt”, “broadcastList.txt”. Noted that “broadcastList.txt” is the only txt file end with 2 ‘\n’, other files only end with 1 ‘/n’
(8) Input files for “bidder”: bidderPass1.txt, bidderPass2.txt, bidding1.txt, bidding2.txt
(9) Input files for “seller”: sellerPass1.txt, sellerPass2.txt, itemList1.txt, itemList2.txt
(10) My code use sleep() to synchronize 3 phases among auctionserver, bidder and seller. Run 3 programs in the order of 1. auctionserver, 2.bidder, 3.seller. And try to minimize the intervals, like within 5 seconds.
(11) All the packets exchanged starts with the sender identifier, for example “<Bidder1>Login#bla bla bla”. The identifiers will be removed before printed on screen.

h. Reused code:
The use of socket functions like "socket", "bind", "connect", "send", "recv" etc are referenced from Beej's guide to socket programming, and the include files are copied from examples in the same guide.