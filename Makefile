CC = gcc
FLAGS = -g -pthread -lrt -Wall
OBJS = Functions.c HeaderFile.h
MAIN = SystemManager.c
MOBILEUSER = MobileUser.c
BACKOFFICEUSER = BackOfficeUser.c
TARGET = 5g_auth_platform
TARGET2 = mobile_user
TARGET3 = backoffice_user
LOG = ./files/logFile.txt

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(FLAGS) $(MAIN) $(OBJS) -g -o $(TARGET) -lrt
	$(CC) $(FLAGS) $(MOBILEUSER) $(OBJS) -g -o $(TARGET2) -lrt
	$(CC) $(FLAGS) $(BACKOFFICEUSER) $(OBJS) -g -o $(TARGET3) -lrt

clean:
	rm -f $(TARGET) $(TARGET2) $(TARGET3) $(LOG)