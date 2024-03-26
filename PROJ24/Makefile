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

all: $(TARGET) $(TARGET2) $(TARGET3)

$(TARGET): $(OBJS) $(MAIN)
	$(CC) $(FLAGS) $(MAIN) $(OBJS) -o $(TARGET) -lrt

$(TARGET2): $(OBJS) $(MOBILEUSER)
	$(CC) $(FLAGS) $(MOBILEUSER) $(OBJS) -o $(TARGET2) -lrt

$(TARGET3): $(OBJS) $(BACKOFFICEUSER)
	$(CC) $(FLAGS) $(BACKOFFICEUSER) $(OBJS) -o $(TARGET3) -lrt

clean:
	rm -f $(TARGET) $(TARGET2) $(TARGET3) $(LOG)