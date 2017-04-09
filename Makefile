
CFLAGS =	-std=gnu99 -Wall -Wextra -Werror -Wno-unused-parameter

BOARDS =	sentry

OUTPUTS =	$(BOARDS:%=output/%.conf)

all: tool $(OUTPUTS)

tool: src/tool.c src/custr.c
	gcc $(CFLAGS) -o $@ $^

output/%.conf: %/dfw_sst.bin header.conf tool
	cat header.conf > $@
	./tool $< >> $@

