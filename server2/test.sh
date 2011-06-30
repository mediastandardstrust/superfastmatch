function send_doc {
	method=$1
	doctype=$2
	docid=1
	files=$3
	
	for file in $files
	do
		if [[ "${method}" == "DELETE" ]] ; then
			echo "curl -X $method -H \"Expect:\" 127.0.0.1:1978/document/$doctype/$docid/"
			curl -sS -X $method -H "Expect:" 127.0.0.1:1978/document/$doctype/$docid/ -o test.log #&			
		else
			# echo "$(date) $method-ing $file with doctype: $doctype docid: $docid"
			echo "curl -X $method -H \"Expect:\" -d \"filename=$file\" --data-urlencode \"text@$file\" 127.0.0.1:1978/document/$doctype/$docid/"
			curl -sS -X $method -H "Expect:" -d "filename=$file" --data-urlencode "text@$file" 127.0.0.1:1978/document/$doctype/$docid/ -o test.log #&
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
# send_doc POST 2 "../superfastmatch/fixtures/*.txt" 

# echo "Test POST-ing documents"
# send_doc POST 2 "../superfastmatch/fixtures/*.txt"
# 
# echo "Test DELETE-ing documents"
# send_doc DELETE 2  "../superfastmatch/fixtures/*.txt" 

# echo "Test batch indexing"
# curl -X POST -H "Expect:" 127.0.0.1:1978/index/ 

echo "Test POST-ing documents"
send_doc POST 1  "../superfastmatch/fixtures/pan11-external/source-documents/*.txt"

# echo "Test POST-ing documents"
# send_doc POST 2  "../superfastmatch/fixtures/pan11-external/suspicious-documents/*.txt"

# 
# echo "Test batch indexing"
# curl -X POST -H "Expect:" 127.0.0.1:1978/index/


# echo "Test PUT-ing documents"
# send_doc PUT 3  "../superfastmatch/fixtures/pan11-external/source-documents/*.txt"