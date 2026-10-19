CC:=gcc

virtual_memory_test: virtual_memory_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

execve_invalid_path: execve_invalid_path.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

execve_full_path:  execve_full_path.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

process_group_block_test: process_group_block_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@


process_group_father_son_test: process_group_father_son_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@