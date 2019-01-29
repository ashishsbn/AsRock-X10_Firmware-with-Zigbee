
ASROCK_ROUTER_FOLDER = src

.PHONY: all clean asrock_mqttd

all: asrock_mqttd

asrock_mqttd:
	echo "make -C $(ASROCK_ROUTER_FOLDER) all"
	make -C $(ASROCK_ROUTER_FOLDER) all

clean:
	make -C $(ASROCK_ROUTER_FOLDER) clean
