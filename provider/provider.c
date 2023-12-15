#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <inttypes.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdbool.h>

static void signal_handler(int signum);
static uint64_t do_receive(mqd_t mq);

enum MQ {
	MQ_DEFAULT,
	MQ_RO,
	MQ_RW,
	MQ_ALL_PERMS,
	MQ_NUM,
};

const char *names[] = {
	[MQ_DEFAULT] = "/test-default",
	[MQ_RO] = "/test-ro",
	[MQ_RW] = "/test-rw",
	[MQ_ALL_PERMS] = "/test-all-perms",
};

static atomic_bool _run = true;

static void signal_handler(int signum)
{
	if (signum == SIGINT) {
		puts("Terminating...");
		_run = false;
	}
}

static uint64_t do_receive(mqd_t mq)
{
	ssize_t ret;
	static const struct timespec timeout = {
		.tv_nsec = 1000000,
		.tv_sec = 0,
	};
	uint64_t data;

	ret = mq_timedreceive(mq, (char *)&data, sizeof(data), NULL, &timeout);
	if (ret < 0) {
		if (errno != ETIMEDOUT) {
			printf("mq_timedreceive failed: %s\n", strerror(errno));
		}
		return 0;
	}

	return data;
}

int main(void)
{
	mqd_t mqs[MQ_NUM] = { 0 };

	for (int i = 0; i < MQ_NUM; i++) {
		struct mq_attr attr = {
			.mq_msgsize = sizeof(uint64_t),
			.mq_maxmsg = 8,
		};

		mqs[i] = mq_open(names[i], O_CREAT | O_RDWR | O_EXCL,
				 DEFFILEMODE, &attr);
		if (mqs[i] < 0) {
			printf("MQ \"%s\" failed: %s\n", names[i],
			       strerror(errno));
			exit(-errno);
		}

		printf("MQ \"%s\" created\n", names[i]);
	}

	signal(SIGINT, signal_handler);
	while (_run) {
		for (int i = 0; i < MQ_NUM; i++) {
			uint64_t data = do_receive(mqs[i]);
			if (data != 0) {
				printf("MQ \"%s\": 0x%" PRIX64 "\n", names[i],
				       data);
			}
		}
	}

	for (int i = 0; i < MQ_NUM; i++) {
		mq_close(mqs[i]);
	}
}