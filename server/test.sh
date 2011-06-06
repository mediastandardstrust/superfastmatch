for f in ../superfastmatch/fixtures/*.txt
do
	echo "Adding $f"
	curl  --data-urlencode "name=add" \
	 	  --data-urlencode "_docid=1" \
	 	  --data-urlencode "_doctype=1" \
	      --data-urlencode "_windowsize=15" \
	      --data-urlencode "_text@$f" \
	      127.0.0.1:1978/rpc/play_script &
    NPROC=$(($NPROC+1))
    if [ "$NPROC" -ge 8 ]; then
        wait
        NPROC=0
    fi
done
