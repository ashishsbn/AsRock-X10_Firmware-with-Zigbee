#!/bin/sh

do_syslog()
{
	logger -p user.info -t "zstack_cgi" "$*"
}

do_syslog_err()
{
	logger -p user.error -t "zstack_cgi" "$*"
}

do_run()
{
	do_syslog $@
	$@ > /dev/null 2>&1
	if [ $? -ne 0 ];then
		do_syslog_err FAILED: $@
	fi
}
