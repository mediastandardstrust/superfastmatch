function send_doc {
	method=$1
	doctype=$2
	docid=1
	for file in ../superfastmatch/fixtures/*.txt
	do
		echo "$(date) Adding $file with doctype: $doctype docid: $docid"
		curl -X $method -H "Expect:" -d "filename=$file" --data-urlencode "text@$file" 127.0.0.1:1978/document/$doctype/$docid/ #&
		docid=$(($docid+1))
	    NPROC=$(($NPROC+1))
	    # if [ "$NPROC" -ge 4 ]; then
	    #     wait
	    #     NPROC=0
	    # fi
	done
	wait
}

echo "Test PUT-ing documents as doctype 1"
send_doc PUT 1

echo "Test PUT-ing documents as doctype 2"
send_doc PUT 2

# echo "Test POST-ing documents"
# send_doc POST 3