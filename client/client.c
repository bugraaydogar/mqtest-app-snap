#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

int test_mq(const char *name, int flags);

enum test {
	TEST_RW,
	TEST_DEFAULT,
	TEST_RO,
	TEST_ALL_PERMS,
	TEST_INVALID,
};

static const char *const permissions[] = {
	[TEST_RW] = "rw",
	[TEST_DEFAULT] = "default",
	[TEST_RO] = "ro",
	[TEST_ALL_PERMS] = "all",
};

int test_mq(const char *name, int flags)
{
	uint64_t data = 0xDEADBEEF;
	mqd_t mq = mq_open(name, flags);
	if (mq < 0) {
		printf("MQ \"%s\" failed: %s\n", name, strerror(errno));
		return -errno;
	}

	if (mq_send(mq, (char *)&data, sizeof(data), 0) < 0) {
		printf("MQ \"%s\" mq_send failed: %s\n", name, strerror(errno));
		return -errno;
	}

	mq_close(mq);

	return 0;
}

void print_usage(char *argv[])
{
	printf("Usage: %s <permissions>\n", argv[0]);
	puts("<permissions> can be one of:");
	for (int i = 0; i < TEST_INVALID; i++) {
		printf("  - %s\n", permissions[i]);
	}
}

int main(int argc, char *argv[])
{
	enum test test_id = TEST_INVALID;
	int ret = 0;

	if (argc != 2) {
		print_usage(argv);
		return -1;
	}

	for (int i = 0; i < TEST_INVALID; i++) {
		if (!strcmp(permissions[i], argv[1])) {
			test_id = i;
		}
	}

	switch (test_id) {
	case TEST_RW:
		ret = test_mq("/test-rw", O_RDWR);
		break;
	case TEST_DEFAULT:
		ret = test_mq("/test-default", O_RDWR);
		break;
	case TEST_RO:
		/* read-only mode should fail with -EPERM */
		ret = test_mq("/test-ro", O_RDONLY) == -EPERM ? 0 : -1;
		break;
	case TEST_ALL_PERMS:
		ret = test_mq("/test-all-perms", O_RDWR);
		break;
	case TEST_INVALID:
	default:
		printf("Invalid argument: %s\n", argv[1]);
		print_usage(argv);
		return -1;
	}

	printf("Test %s\n", ret ? "failed" : "succeeded");

	return ret;
}