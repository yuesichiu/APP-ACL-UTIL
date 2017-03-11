default:
	gcc -g -o app-acl-util app-acl-util.h app-acl-util.c md5.h md5.c -lpthread -lsqlite3

clean:
	rm -f *.o app-acl-util	
