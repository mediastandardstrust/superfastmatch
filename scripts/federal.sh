curl -X POST -H "Expect:" -d "title=7K5.txt" --data-urlencode "text@fixtures/federal/7K5.txt" 127.0.0.1:8080/document/1/1/
curl -X POST -H "Expect:" -d "title=SB 1070.txt" --data-urlencode "text@fixtures/federal/SB 1070.txt" 127.0.0.1:8080/document/1/2/
curl -X POST  127.0.0.1:8080/association/