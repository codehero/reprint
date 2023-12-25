#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <unistd.h>

#include <reprint/reprint.h>

int main(int argc, const char* argv[]){
	if(argc < 3){
		fprintf(stderr, "Usage: %s DATA_LEN EXPR\n", argv[0]);
		return 1;
	}

	/* Get expected length of data. */
	unsigned len = strtoul(argv[1], NULL, 10);

	/* Just limit to 8k, for the hell of it and my lack of time. */
	if(len > 8192)
		return 2;

	/* Read data from stdin. */
	uint8_t buffer[len];
	unsigned acc = 0;
	while(acc < len){
		int more = read(0, buffer + acc, len - acc);
		if(more < 0)
			return 3;

		if(more == 0)
			return 4;

		acc += more;
	}
	close(0);


	reprint_state rs;
	reprint_init(&rs, argv[2], buffer);

	uint8_t output[8192];
	int ret = reprint_cb(&rs, output, sizeof(output));
	if(ret < 0)
		return 5;

	/* This is supposed to be simple....just bail on overlong output */
	if(sizeof(output) == ret)
		return 6;

	if(write(1, output, ret))
		return 7;

	return 0;
}
