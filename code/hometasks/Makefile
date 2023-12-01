CC = gcc
CFLAGS = -Wall -Werror -Wextra

SOURCES_QUEST_1 = useless.c
OBJECTS_QUEST_1 = $(SOURCES_QUEST_1:.c=.o)

SOURCES_QUEST_2 = backup.c
OBJECTS_QUEST_2 = $(SOURCES_QUEST_2:.c=.o)

SOURCES_QUEST_3 = runsim.c
OBJECTS_QUEST_3 = $(SOURCES_QUEST_3:.c=.o)
 
TARGET_NAME_QUEST_1 = ./build/useless
TARGET_NAME_QUEST_2 = ./build/backup
TARGET_NAME_QUEST_3 = ./build/runsim


all: useless backup
 
.c.o:
	$(CC) $(CFLAGS) -c $< -o $@
 
clean:
	rm -rf *.o ../build/
 
rebuild: clean all

useless: $(OBJECTS_QUEST_1)
	$(CC) $(CFLAGS) -o $(TARGET_NAME_QUEST_1) $(OBJECTS_QUEST_1)

backup:$(OBJECTS_QUEST_2)
	$(CC) $(CFLAGS) -o $(TARGET_NAME_QUEST_2) $(OBJECTS_QUEST_2) -lm -lz

runsim: $(OBJECTS_QUEST_3)
	$(CC) $(CFLAGS) -o $(TARGET_NAME_QUEST_3) $(OBJECTS_QUEST_3)