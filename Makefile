
CFLAGS =	-std=gnu99 -Wall -Wextra -Werror -Wno-unused-parameter

BOARDS =	sentry

OUTPUTS =	$(BOARDS:%=output/%.conf)

all: tool $(OUTPUTS)

tool: src/tool.c src/custr.c
	gcc $(CFLAGS) -o $@ $^

output/%.conf: %/dfw_sst.bin header.conf tool
	mkdir -p $(@D)
	cat header.conf > $@
	./tool $< >> $@

clean:
	rm -f tool output/*.conf


