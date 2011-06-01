superfastmatch=require('superfastmatch')

function load(filename)
   return assert(io.open(filename, "r")):read("*all"):lower():gsub('\n',' ')
end

docs = {	
			Document(1,1,load("../superfastmatch/fixtures/oliver_twist.txt"),15),
			Document(1,2,load("../superfastmatch/fixtures/great_expectations.txt"),15),
			Document(1,3,load("../superfastmatch/fixtures/koran.txt"),15),
			Document(1,4,load("../superfastmatch/fixtures/bible.txt"),15),
			Document(1,5,load("../superfastmatch/fixtures/bible.txt"),15),
			Document(1,6,load("../superfastmatch/fixtures/bible.txt"),15),
			Document(1,7,load("../superfastmatch/fixtures/bible.txt"),15),
			Document(1,8,load("../superfastmatch/fixtures/bible.txt"),15)
	   }

start = os.clock()
hashes ={}
for i,doc in ipairs(docs) do
	table.insert(hashes,doc:hashes())
	print (#hashes[i])
end
print(string.format("%.2f secs\n", os.clock() - start))
