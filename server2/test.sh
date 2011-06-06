docid=1
for file in ../superfastmatch/fixtures/*.txt
do
	echo "Adding $file with docid: $docid"
	curl -H "Expect:" --data-binary "@$file" 127.0.0.1:1978/document/1/$docid/ &
	docid=$(($docid+1))
    NPROC=$(($NPROC+1))
    if [ "$NPROC" -ge 8 ]; then
        wait
        NPROC=0
    fi
done
