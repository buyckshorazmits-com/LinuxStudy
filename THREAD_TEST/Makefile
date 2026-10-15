CC:=gcc

create_test: create_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

terminal_test: terminate_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

pthread_detach: pthread_detach.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

pthread_cancel_deffered_test: pthread_cancel_deffered_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

pthread_cancel_async_test: pthread_cancel_async_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

pthread_cancel_disabled_test: pthread_cancel_disabled_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

pthread_cancel_pending_test: pthread_cancel_pending_test.c
	-$(CC) -o $@ $^ -pthread
	-stdbuf -o0 ./$@
	-rm ./$@

race_condition_test: race_condition_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

mutex_test: mutex_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

rwlock_test_writer_unlock: rwlock_test_writer_unlock.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

rwlock_test: rwlock_test.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

rwlock_rw_alternate: rwlock_rw_alternate.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

rwlock_write_hungry: rwlock_write_hungry.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

rwlock_hungry_solved: rwlock_hungry_solved.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

# condition_var 是永不退出的死循环 demo:Ctrl+C 中断时 rm 执行不到,残留的二进制会被 make 当成"已是最新"而跳过一切
# 声明为伪目标(.PHONY)后,每次 make 都会强制重新编译并运行
.PHONY: condition_var
condition_var: condition_var.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

unnamed_sem_bin_thread_condition: unnamed_sem_bin_thread_condition.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

unnamed_sem_bin_thread: unnamed_sem_bin_thread.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

unnamed_sem_bin_process_condition: unnamed_sem_bin_process_condition.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

unnamed_sem_bin_process: unnamed_sem_bin_process.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@


unnamed_sem_bin_process_illegal: unnamed_sem_bin_process_illegal.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

unnamed_sem_bin_process_isolate: unnamed_sem_bin_process_isolate.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

unnamed_sem_count_thread: unnamed_sem_count_thread.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

unnamed_sem_count_process: unnamed_sem_count_process.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

named_sem_bin: named_sem_bin.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

named_sem_count: named_sem_count.c
	-$(CC) -o $@ $^
	-./$@
	-rm ./$@

named_sem_no_unlink: named_sem_no_unlink.c
	-$(CC) -o $@ $^
	-./$@ 
	-rm ./$@

thread_pool_test: thread_pool_test.c
	-$(CC) -o $@ $^ `pkg-config --cflags --libs glib-2.0`
	-./$@ 
	-rm ./$@


	
