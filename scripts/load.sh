function send_doc {
	method=$1
	doctype=$2
	docid=1
	dir=$3
	filemask=$4
	
	# Smallest first
	for file in `ls -ASr $dir | grep $filemask`
	# Largest first
	# for file in `ls -AS $dir | grep $filemask`
	# Unsorted
	# for file in `ls -A $dir | grep $filemask`
	do
		if [[ "${method}" == "DELETE" ]] ; then
			echo "curl -X $method -H \"Expect:\" 127.0.0.1:8080/document/$doctype/$docid/"
			curl -sS -X $method -H "Expect:" 127.0.0.1:8080/document/$doctype/$docid/ -o test.log &
		else
			echo "curl -X $method -H \"Expect:\" -d \"title=$file\" --data-urlencode \"text@$dir$file\" 127.0.0.1:8080/document/$doctype/$docid/"
			curl -sS -X $method -H "Expect:" -d "title=$file" --data-urlencode "text@$dir$file" 127.0.0.1:8080/document/$doctype/$docid/ -o load.log &
		fi
		docid=$(($docid+1))
		NPROC=$(($NPROC+1))
		if [ "$NPROC" -ge 4 ]; then
			  wait
			  NPROC=0
		fi
	done
	wait
}

# echo "Test PUT-ing documents as doctype 1"
# send_doc PUT 1 "fixtures/gutenberg/" ".txt"
 
# echo "Test DELETE-ing documents"
# send_doc DELETE 1 "fixtures/gutenberg/" ".txt"
#  
# echo "Test POST-ing documents"
# send_doc POST 2 "fixtures/gutenberg/" ".txt"
# 
# echo "Test batch indexing"
# curl -X POST -H "Expect:" 127.0.0.1:8080/association/
# 

# echo "Test POST-ing documents"
# send_doc POST 1 "fixtures/congressional-record/" ".txt"


# # echo "Test POST-ing documents"
send_doc POST 1 "fixtures/pan11-external/source-documents/" ".txt"

# echo "Test POST-ing documents"
# send_doc POST 2 "fixtures/pan11-external/suspicious-documents/" ".txt"

echo "Test batch indexing"
curl -X POST -H "Expect:" 127.0.0.1:8080/association/