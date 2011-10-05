curl -X POST -H "Expect:" --data-urlencode "text@fixtures/gutenberg/bleak_house.txt" -d "title=Bleak+House&author=Charles+Dickens" 127.0.0.1:8080/document/1/1/
curl -X POST -H "Expect:" --data-urlencode "text@fixtures/gutenberg/great_expectations.txt" -d "title=Great+Expectations&author=Charles+Dickens" 127.0.0.1:8080/document/1/2/
curl -X POST -H "Expect:" --data-urlencode "text@fixtures/gutenberg/little_dorrit.txt" -d "title=Little+Dorrit&author=Charles+Dickens" 127.0.0.1:8080/document/1/3/
curl -X POST -H "Expect:" --data-urlencode "text@fixtures/gutenberg/christmas_carol.txt" -d "title=Christmas+Carol&author=Charles+Dickens" 127.0.0.1:8080/document/1/4/
curl -X POST -H "Expect:" --data-urlencode "text@fixtures/gutenberg/hard_times.txt" -d "title=Hard+Times&author=Charles+Dickens" 127.0.0.1:8080/document/1/5/
curl -X POST -H "Expect:" --data-urlencode "text@fixtures/gutenberg/old_curiosity_shop.txt" -d "title=Old+Curiosity+Shop&author=Charles+Dickens" 127.0.0.1:8080/document/1/6/
curl -X POST -H "Expect:" --data-urlencode "text@fixtures/gutenberg/tale_of_two_cities.txt" -d "title=Tale+of+two+cities&author=Charles+Dickens" 127.0.0.1:8080/document/1/7/
curl -X POST -H "Expect:" --data-urlencode "text@fixtures/gutenberg/david_copperfield.txt" -d "title=David+Copperfield&author=Charles+Dickens" 127.0.0.1:8080/document/1/8/
curl -X POST -H "Expect:" --data-urlencode "text@fixtures/gutenberg/oliver_twist.txt" -d "title=Oliver+Twist&author=Charles+Dickens" 127.0.0.1:8080/document/1/9/
curl -X POST -H "Expect:" --data-urlencode "text@fixtures/gutenberg/koran.txt" -d "title=Koran&author=Various" 127.0.0.1:8080/document/2/1/
curl -X POST -H "Expect:" --data-urlencode "text@fixtures/gutenberg/bible.txt" -d "title=Bible&author=Various" 127.0.0.1:8080/document/2/2/
curl -X POST -H "Expect:" 127.0.0.1:8080/association
