superfastmatch=require('superfastmatch')

function load(filename)
   return assert(io.open(filename, "r")):read("*all"):lower():gsub('\n',' ')
end

windowsize=20
subject=3

docs = {	
			Document(1,1,load("../superfastmatch/fixtures/oliver_twist.txt"),windowsize),
			Document(1,2,load("../superfastmatch/fixtures/great_expectations.txt"),windowsize),
			Document(1,3,load("../superfastmatch/fixtures/david_copperfield.txt"),windowsize),
			Document(1,4,load("../superfastmatch/fixtures/christmas_carol.txt"),windowsize),
			Document(1,5,load("../superfastmatch/fixtures/bleak_house.txt"),windowsize),
			Document(1,6,load("../superfastmatch/fixtures/pickwick_papers.txt"),windowsize),
			Document(1,7,load("../superfastmatch/fixtures/hard_times.txt"),windowsize),
			Document(1,8,load("../superfastmatch/fixtures/little_dorrit.txt"),windowsize),
			Document(1,9,load("../superfastmatch/fixtures/old_curiosity_shop.txt"),windowsize),
			Document(1,10,load("../superfastmatch/fixtures/tale_of_two_cities.txt"),windowsize),
			Document(1,11,load("../superfastmatch/fixtures/oliver_twist.txt"),windowsize),
			Document(1,12,load("../superfastmatch/fixtures/oliver_twist.txt"),windowsize),
			Document(1,13,load("../superfastmatch/fixtures/oliver_twist.txt"),windowsize),
			Document(1,14,load("../superfastmatch/fixtures/oliver_twist.txt"),windowsize),
			Document(1,15,load("../superfastmatch/fixtures/oliver_twist.txt"),windowsize),
			Document(1,16,load("../superfastmatch/fixtures/oliver_twist.txt"),windowsize),
			Document(1,17,load("../superfastmatch/fixtures/oliver_twist.txt"),windowsize),
			Document(1,18,load("../superfastmatch/fixtures/oliver_twist.txt"),windowsize),
			Document(1,19,load("../superfastmatch/fixtures/koran.txt"),windowsize),
			Document(1,20,load("../superfastmatch/fixtures/bible.txt"),windowsize)
	   }

start = os.clock()
-- hashes ={}
-- for i,doc in ipairs(docs) do
-- 	table.insert(hashes,doc:hashes())
-- 	print (#hashes[i])
-- end

matcher=Matcher()
for i,doc in ipairs(docs) do
	if i~=subject then
		matcher:match(docs[subject],doc)
	end
end
print(string.format("%.2f secs\n", os.clock() - start))
