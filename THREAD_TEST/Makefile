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