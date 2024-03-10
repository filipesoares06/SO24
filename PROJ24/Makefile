CC = gcc
FLAGS = -g -pthread -lrt -Wall
OBJS = Functions.c HeaderFile.h
MAIN = SystemManager.c
MOBILEUSER = MobileUser.c
TARGET = system
TARGET2 = mobileuser
LOG = ./files/log.txt

all: $(TARGET)

$(TARGET): $(OBJS)
		$(CC) $(FLAGS) $(MAIN) $(OBJS) -g -o  $(TARGET) -lrt
		$(CC) $(FLAGS) $(MOBILEUSER) $(OBJS) -g -o $(TARGET2) -lrt
		
clean:
	$(RM) $(TARGET) $(TARGET2) $(LOG)