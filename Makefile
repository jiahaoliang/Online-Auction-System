auctionserver: auctionserver.o singlyLinkedList.o
	gcc -o auctionserver auctionserver.o singlyLinkedList.o -lsocket -lnsl -lresolv -g

auctionserver.o: auctionserver.c singlyLinkedList.h
	gcc -g -c -Wall auctionserver.c

singlyLinkedList.o: singlyLinkedList.c singlyLinkedList.h
	gcc -g -c -Wall singlyLinkedList.c

clean:
	rm -f *.o auctionserver
