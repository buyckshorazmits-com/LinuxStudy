CC:=gcc

.PHONY: single_conn single_conn_clean

single_conn_executables:=single_conn_server single_conn_client

num_endianess_convert: num_endianess_convert.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

inet_endian_convert: inet_endian_convert.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

single_conn_server: single_conn_server.c
	-$(CC) -o $@ $^

single_conn_client: single_conn_client.c
	-$(CC) -o $@ $^

single_conn: $(single_conn_executables)

single_conn_clean:
	-rm ./$(word 1, $(single_conn_executables)) ./$(word 2, $(single_conn_executables))

without_buf_test: without_buf_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

line_buf_test: line_buf_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@
	