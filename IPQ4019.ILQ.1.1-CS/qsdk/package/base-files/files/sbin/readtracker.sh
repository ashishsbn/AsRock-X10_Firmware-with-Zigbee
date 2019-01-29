export VALUE
export NODE=$1

test_lora -LoraMode 0
sleep 1
test_lora -LoraSystemMode 0
sleep 1
test_lora -LoraStartWork 0
sleep 1
test_lora -LoraJoinNode $NODE
sleep 1
if [ -d "/sys/class/gpio/gpio48" ]; then
	echo "directory is exist"
else
	echo 48 > /sys/class/gpio/export
	echo "in" > /sys/class/gpio/gpio48/direction
fi

VALUE=`cat /sys/class/gpio/gpio48/value`

while [ 1 ]
do
	VALUE=`cat /sys/class/gpio/gpio48/value`
	#echo "##VALUE = $VALUE"
	if [ $VALUE -eq 0 ]; then
		test_lora -read 64
		#sleep 1 
		#echo "VALUE## = $VALUE"
		#sleep 1
	fi
done
