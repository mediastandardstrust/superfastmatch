# echo "Test POST-ing documents"
# docid=1
# for file in ../superfastmatch/fixtures/*.txt
# do
# 	echo "$(date) Adding $file with docid: $docid"
# 	curl -H "Expect:" -d "filename=$file" --data-urlencode "text@$file" 127.0.0.1:1978/document/1/$docid/ &
# 	docid=$(($docid+1))
#     NPROC=$(($NPROC+1))
#     if [ "$NPROC" -ge 4 ]; then
#         wait
#         NPROC=0
#     fi
# done

echo "Test PUT-ing documents"
docid=1
for file in ../superfastmatch/fixtures/*.txt
do
	echo "$(date) Adding $file with docid: $docid"
	curl -X PUT -H "Expect:" -d "filename=$file" --data-urlencode "text@$file" 127.0.0.1:1978/document/1/$docid/ #&
	docid=$(($docid+1))
    NPROC=$(($NPROC+1))
    # if [ "$NPROC" -ge 4 ]; then
    #     wait
    #     NPROC=0
    # fi
done
wait