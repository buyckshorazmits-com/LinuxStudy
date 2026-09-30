CC:=gcc

.PHONY: named_fifo named_fifo_clean p_c_mq pc_mq_clean
fopen_test: fopen_test.c
#gcc -o fopen_test fopen_test.c
	-$(CC) -o $@ $^
#./fopen_test
	-./$@
#rm ./fopen_test
	-rm ./$@

fclose_test: fclose_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

fputc_test: fputc_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

fputs_test: fputs_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

fprintf_test: fprintf_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

fgetc_test: fgetc_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

fgets_test: fgets_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

fscanf_test: fscanf_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

fstdin_out_err_test: fstdin_out_err_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

system_call_test: system_call_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

system_tests: system_tests.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

fork_test: fork_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

fork_fd_test: fork_fd_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

erlou: erlou.c
	-$(CC) -o $@ $^
	

execve_test: execve_test.c erlou
	-$(CC) -o $@ $<
	-./$@
	-rm ./$@ ./$(word 2, $^)

fork_execve_test: fork_execve_test.c erlou
	-$(CC) -o $@ $(firstword $^)
	-./$@
	-rm ./$@ ./$(lastword $^)

waitpid_test: waitpid_test.c
	$(CC) -o $@ $^
	-./$@
	-rm ./$@ 

pstree_test: pstree_test.c erlou
	-$(CC) -o $@ $<
	-./$@
	-rm ./$@ ./$(lastword $^)

erlou_block: erlou_block.c
	-$(CC) -o $@ $^

orphan_process_test: orphan_process_test.c erlou_block
	-$(CC) -o $@ $<
	-./$@
	-rm ./$@ ./$(word 2, $^)
unnamed_piped_test: unnamed_piped_test.c
	-$(CC) -o $@ $<
	-./$@ "test"
	-rm ./$@

fifo_read: fifo_read.c
	-$(CC) -o $@ $<

fifo_write: fifo_write.c
	-$(CC) -o $@ $<

named_fifo: fifo_read fifo_write

named_fifo_clean:
	-rm ./fifo_write ./fifo_read

shared_memory: shared_memory.c
	-$(CC) -o $@ $^ 
	-./$@
	-rm ./$@

shared_memory_block: shared_memory_block.c
	-$(CC) -o $@ $^ 
	-./$@
	-rm ./$@

father_son_mq_test: father_son_mq_test.c
	-$(CC) -o $@ $^ 
	-./$@
	-rm ./$@

producer: producer.c
	$(CC) -o $@ $^

consumer: consumer.c
	$(CC) -o $@ $^

p_c_mq: producer consumer

pc_mq_clean:
	-rm ./producer ./consumer

signal_test: signal_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@
