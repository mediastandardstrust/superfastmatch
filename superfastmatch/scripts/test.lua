local superfastmatch = require("superfastmatch")

function load(filename)
   return assert(io.open(filename, "r")):read("*all"):lower():gsub('\n',' ')
end

-- print(superfastmatch.match("Elizabeth, the queen of England sits at her throne","Her majesty the queen of England, is at home",15))
-- print(superfastmatch.match("This string is identical","This string is identical",15))
-- print(superfastmatch.match("10 Downing Street","10 Downing Street Gordon Brown,10 Downing Street",10))



local oliver_twist = load("../fixtures/oliver_twist.txt")
local great_expectations = load("../fixtures/great_expectations.txt")
local bible = load("../fixtures/bible.txt")
local koran = load("../fixtures/koran.txt")

start = os.clock()
print(superfastmatch.match(oliver_twist,great_expectations,20))
print(string.format("elapsed time: %.2f\n", os.clock() - start))

start = os.clock()
print(superfastmatch.match(oliver_twist,bible,25))
print(string.format("elapsed time: %.2f\n", os.clock() - start))

start = os.clock()
print(superfastmatch.match(koran,bible,20))
print(string.format("elapsed time: %.2f\n", os.clock() - start))