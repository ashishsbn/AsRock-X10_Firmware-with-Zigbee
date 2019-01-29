PROG = asrock_mqttd

CXX ?= g++
CC ?= gcc
AR ?= ar

OBJ = asrock_main.o config.o

INC_PATH = -I ./include -I ./mbedtls/include -I . -I /callbacks
LIB_PATH = -L ./library -L ./library/mbedtls
LIB_FLAG = -lasrock_mqttd -lawsmqtt -lmbedtls -lmbedx509 -lmbedcrypto -lpthread -lubox -lubus -lblobmsg_json -luci
SQLITE_LIB = -lsqlite3


DEBUG_FLAG = -DIOT_DEBUG -DIOT_INFO -DIOT_WARN -DIOT_ERROR 
CFLAGS = $(DEBUG_FLAG) -std=gnu99 -Wall -fPIC -pedantic -O2 -D_GNU_SOURCE -DDEBUG_VERBOSE=0 -DDEVICE_OFFLINE=0

all: $(OBJ)
	$(CC) -o $(PROG) $(OBJ) $(INC_PATH) $(LIB_PATH) $(LIB_FLAG) $(SQLITE_LIB) $(CFLAGS)

clean:
	rm -rf *.o *.a $(PROG)

.c.o:
	$(CC) -c $< $(INC_PATH) $(CFLAGS) -o $@
.cpp.o:
	$(CXX) -c $< $(INC_PATH) $(CFLAGS) -o $@

