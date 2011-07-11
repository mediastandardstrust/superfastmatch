function send_doc {
	method=$1
	doctype=$2
	docid=1
	dir=$3
	filemask=$4
	
	for file in `ls -AS $dir | grep $filemask`
	do
		if [[ "${method}" == "DELETE" ]] ; then
			echo "curl -X $method -H \"Expect:\" 192.168.0.3:1978/document/$doctype/$docid/"
			curl -sS -X $method -H "Expect:" 192.168.0.3:1978/document/$doctype/$docid/ -o test.log #&			
		else
			echo "curl -X $method -H \"Expect:\" -d \"title=$file\" --data-urlencode \"text@$dir/$file\" 192.168.0.3:1978/document/$doctype/$docid/"
			curl -sS -X $method -H "Expect:" -d "title=$file" --data-urlencode "text@$dir/$file" 192.168.0.3:1978/document/$doctype/$docid/ -o test.log #&
		fi
		docid=$(($docid+1))
	    NPROC=$(($NPROC+1))
	    # if [ "$NPROC" -ge 4 ]; then
	    #     wait
	    #     NPROC=0
	    # fi
	done
	wait
}

# echo "Test PUT-ing documents as doctype 1"
# send_doc PUT 1
# 
# echo "Test PUT-ing documents as doctype 2"
# send_doc PUT 2 "../superfastmatch/fixtures/*.txt"

# echo "Test POST-ing documents"
# send_doc POST 1 "../superfastmatch/fixtures/*.txt" 
# 
# echo "Test POST-ing documents"
# send_doc POST 2 "../superfastmatch/fixtures/*.txt"
# 
# echo "Test DELETE-ing documents"
# send_doc DELETE 2  "../superfastmatch/fixtures/*.txt" 

# echo "Test batch indexing"
# curl -X POST -H "Expect:" 127.0.0.1:1978/index/ 
# 
echo "Test POST-ing documents"
send_doc POST 1  "../superfastmatch/fixtures/pan11-external/source-documents/" ".txt"

echo "Test POST-ing documents"
send_doc POST 2  "../superfastmatch/fixtures/pan11-external/suspicious-documents/" ".txt"

# 
# echo "Test batch indexing"
# curl -X POST -H "Expect:" 127.0.0.1:1978/index/


# echo "Test PUT-ing documents"
# send_doc PUT 3  "../superfastmatch/fixtures/pan11-external/source-documents/*.txt"