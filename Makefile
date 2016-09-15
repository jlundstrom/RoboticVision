TARGET := pgm

BUILD := build
SRCS := pgm.c filter.c sobel.c canny.c readPGM.c
OBJS := $(patsubst %,$(BUILD)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

CC := gcc
LD := gcc

CFLAGS += -Wall -Werror -ggdb
LDFLAGS += -ggdb
LDLIBS += -lm


all: $(TARGET)

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)


$(BUILD)/%.o: % $(BUILD)/.dir
	$(CC) $(CFLAGS) -c -MD -MP -MF $(BUILD)/$*.d -o $@ $<

-include $(DEPS)

%.dir:
	mkdir -p $(@D) && touch $@


clean:
	rm -rf $(BUILD)

.SECONDARY:

.PHONY: all clean
