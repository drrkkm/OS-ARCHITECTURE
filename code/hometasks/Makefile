CC = gcc
CFLAGS = -Wall -Werror -Wextra

SOURCES_QUEST_1 = ./useless/useless.c
OBJECTS_QUEST_1 = $(SOURCES_QUEST_1:.c=.o)

SOURCES_QUEST_0 = ./useless/test.c
OBJECTS_QUEST_0 = $(SOURCES_QUEST_0:.c=.o)

SOURCES_QUEST_2 = ./backup/backup.c
OBJECTS_QUEST_2 = $(SOURCES_QUEST_2:.c=.o)

SOURCES_QUEST_3 = ./runsim/runsim.c
OBJECTS_QUEST_3 = $(SOURCES_QUEST_3:.c=.o)

SOURCES_QUEST_4 = ./runsim/sleep.c
OBJECTS_QUEST_4 = $(SOURCES_QUEST_4:.c=.o)

SOURCES_QUEST_5 = ./dish/dishwasher.c
OBJECTS_QUEST_5 = $(SOURCES_QUEST_5:.c=.o)

SOURCES_QUEST_6 = ./dish/dishwiper.c
OBJECTS_QUEST_6 = $(SOURCES_QUEST_6:.c=.o)
 
TARGET_NAME_QUEST_1 = ./build/useless
TARGET_NAME_QUEST_0 = ./build/test
TARGET_NAME_QUEST_2 = ./build/backup
TARGET_NAME_QUEST_3 = ./build/runsim
TARGET_NAME_QUEST_4 = ./build/sleep
TARGET_NAME_QUEST_5 = ./dish/dishwasher
TARGET_NAME_QUEST_6 = ./dish/dishwiper


all: useless backup runsim test sleep dishwasher dishwiper
 
.c.o:
	$(CC) $(CFLAGS) -c $< -o $@
 
clean:
	rm -rf */*.o 
 
rebuild: clean all

test: $(OBJECTS_QUEST_0)
	$(CC) $(CFLAGS) -o $(TARGET_NAME_QUEST_0) $(OBJECTS_QUEST_0)

useless: $(OBJECTS_QUEST_1) test
	$(CC) $(CFLAGS) -o $(TARGET_NAME_QUEST_1) $(OBJECTS_QUEST_1)

backup:$(OBJECTS_QUEST_2)
	$(CC) $(CFLAGS) -o $(TARGET_NAME_QUEST_2) $(OBJECTS_QUEST_2) -lm -lz

sleep: $(OBJECTS_QUEST_4)
	$(CC) $(CFLAGS) -o $(TARGET_NAME_QUEST_4) $(OBJECTS_QUEST_4)

runsim: $(OBJECTS_QUEST_3) sleep
	$(CC) $(CFLAGS) -o $(TARGET_NAME_QUEST_3) $(OBJECTS_QUEST_3)

dishwasher: $(OBJECTS_QUEST_5)
	$(CC) $(CFLAGS) -o $(TARGET_NAME_QUEST_5) $(OBJECTS_QUEST_5) -lpthread -lrt

dishwiper: $(OBJECTS_QUEST_6)
	$(CC) $(CFLAGS) -o $(TARGET_NAME_QUEST_6) $(OBJECTS_QUEST_6) -lpthread -lrt