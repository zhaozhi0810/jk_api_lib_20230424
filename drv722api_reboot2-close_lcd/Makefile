CROSS_COMPILE = aarch64-linux-gnu-
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)gcc

SRCS = $(wildcard ./*.c)
OBJS = $(patsubst %.c,%.o,$(SRCS))

CFLAGS = -c -Wall
CFLAGS += -I./
CFLAGS += -I../drv_api/

LDFLAGS = -L../drv_api/
LDFLAGS += -ldrv722_22134
#LDFLAGS += -lpthread

TARGET = hj_reboot

all: $(TARGET)

$(TARGET): $(OBJS)
	$(LD) $(OBJS) -o $@ $(LDFLAGS)

$(OBJS): %.o:%.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(OBJS)
	