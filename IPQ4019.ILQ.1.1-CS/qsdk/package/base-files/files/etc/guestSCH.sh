if [ $1 -eq 2 ];then
	uci set schedule.ath02.enable=0
	uci commit schedule
	sleep 1
	uci set wireless.ath02.disabled=1
	uci commit wireless
	ifconfig ath02 down
	echo "guest network ath02 schedule off!!"
fi
if [ $1 -eq 5 ];then
	uci set schedule.ath12.enable=0
	uci commit schedule
	sleep 1
	uci set wireless.ath12.disabled=1
	uci commit wireless
	ifconfig ath12 down
	echo "guest network ath12 schedule off!!"
fi
