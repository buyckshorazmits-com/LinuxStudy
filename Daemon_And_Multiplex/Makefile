.PHONY: tcp_clean
tcp_executables:=tcp_server tcp_client
CC:=gcc
CFLAGS:=-g -Wall

tcp_server: tcp_server.c
	-$(CC) $(CFLAGS) -o $@ $^

tcp_client: tcp_client.c
	-$(CC) $(CFLAGS) -o $@ $^
tcp_clean:
	-rm ./$(word 1,$(tcp_executables)) ./$(word 2,$(tcp_executables))

daemon_test: daemon_test.c tcp_clean tcp_server tcp_client
	-$(CC) $(CFLAGS) -o $@ $<
	-./$@
	-rm ./$@

epoll_test: epoll_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@
