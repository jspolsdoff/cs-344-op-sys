# By James Spolsdoff

# import needed modules
import random
import string

# for loop to create three files and write to them
for i in range(1, 4):

	# create a file with a ending
	file = "random_file_name" + str(i)

	# open the file
	f = open(file, 'w')

	# variable for contents of file
	random_file_content = ""

	# loop to write 10 random characters in file
	for j in range(0, 10):

			random_file_content += random.choice(string.ascii_lowercase)

	# write the string to the file
	f.write(random_file_content)

	# close file
	f.close()

	#print out contents of random string written to file
	print(random_file_content)

# generate two random integers between 1 and 42
first_num = random.randint(1, 42)
second_num = random.randint(1,42)

# print the numbers
print(first_num)
print(second_num)

# calculate product and print
product = first_num * second_num
print(product)			
