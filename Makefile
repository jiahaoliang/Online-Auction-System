all: auctionserver bidder

auctionserver: auctionserver.o singlyLinkedList.o EE450.o
	gcc -o auctionserver auctionserver.o singlyLinkedList.o -lsocket -lnsl -lresolv -g
	
bidder: bidder.o singlyLinkedList.o EE450.o
	gcc -o bidder bidder.o singlyLinkedList.o -lsocket -lnsl -lresolv -g

auctionserver.o: auctionserver.c singlyLinkedList.h EE450.h
	gcc -g -c -Wall auctionserver.c
	
bidder.o: bidder.c singlyLinkedList.h EE450.h
	gcc -g -c -Wall bidder.c

singlyLinkedList.o: singlyLinkedList.c singlyLinkedList.h
	gcc -g -c -Wall singlyLinkedList.c

EE450.o: EE450.c EE450.h
	gcc -g -c -Wall EE450.c

clean:
	rm -f *.o auctionserver bidder
