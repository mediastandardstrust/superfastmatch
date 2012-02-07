#! /bin/sh
### BEGIN INIT INFO
# Provides:          ft.churnalism.com
# Required-Start:    $local_fs $remote_fs $network $syslog
# Required-Stop:     $local_fs $remote_fs $network $syslog
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# X-Interactive:     true
# Short-Description: Start/stop superfastmatch server example
### END INIT INFO
#----------------------------------------------------------------
# Startup script for Superfastmatch
#----------------------------------------------------------------


# configuration variables
prog="superfastmatch"
cmd="/path/to/superfastmatch/superfastmatch"
data_path="/path/to/hashes/"
port="8080"
slot_count="4"
thread_count="4"
log_file="/path/to/superfastmatch.log"
retval=0

# start the server
start(){
  printf 'Starting the SuperFastMatch server\n'
  mkdir -p "$data_path"
  if ! [ -d "$data_path" ] ; then
    printf 'No such directory: %s\n' "$data_path"
    retval=1
  else
    cmd="$cmd -port $port -slot_count $slot_count -thread_count $thread_count -log_file $log_file -daemonize"
    printf "Executing: %s\n" "$cmd"
    $cmd
    if [ "$?" -eq 0 ] ; then
      printf 'Done\n'
    else
      printf 'The server could not started\n'
      retval=1
    fi
  fi
}

# stop the server
stop(){
  printf 'Stopping the SuperFastMatch server\n'
  fuser -k -s "$log_file"
}

# check permission
if [ -d "$data_path" ] && ! touch "$data_path/$$" >/dev/null 2>&1
then
  printf 'Permission denied\n'
  exit 1
fi
rm -f "$data_path/$$"

# dispatch the command
case "$1" in
start)
  start
  ;;
stop)
  stop
  ;;
restart)
  stop
  start
  ;;
*)
  printf 'Usage: %s {start|stop|restart}\n' "$prog"
  exit 1
  ;;
esac


# exit
exit "$retval"



# END OF FILE
