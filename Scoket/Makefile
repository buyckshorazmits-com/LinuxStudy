CC:=gcc

.PHONY: single_conn single_conn_clean multi_conn_threads multi_conn_threads_clean

single_conn_executables:=single_conn_server single_conn_client
multi_conn_threads_executables:=multi_conn_threads_server multi_conn_threads_client


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

line_buf_with_enter_test: line_buf_with_enter_test.c
	-$(CC) -o $@ $^
	-./$@ 
	-rm ./$@

full_buf_test: full_buf_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

full_buf_fflush_test: full_buf_fflush_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

multi_conn_threads_server: multi_conn_threads_server.c
	-$(CC) -o $@ $^

multi_conn_threads_client: multi_conn_threads_client.c
	-$(CC) -o $@ $^

multi_conn_threads: $(multi_conn_threads_executables)

multi_conn_threads_clean:
	-rm ./$(firstword $(multi_conn_threads_executables)) ./$(lastword $(multi_conn_threads_executables))