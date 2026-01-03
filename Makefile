FLAGS = -O2 -Wall -Wextra
CC = gcc

TARGET_NAME = game2o

SRC  = $(shell find ./src -type f -name *.c)
OBJS = $(SRC:.c=.o)

LIBS = -lraylib -lpthread -lX11 -lGL -lm
all: $(TARGET_NAME)


%.o: %.c
	$(CC) $(FLAGS) -c $< -o $@ 

$(TARGET_NAME): $(OBJS)
	$(CC) $(OBJS) -o $@ $(FLAGS) $(LIBS)

clean:
	rm $(OBJS) $(TARGET_NAME)

.PHONY: all $(TARGET_NAME) clean

