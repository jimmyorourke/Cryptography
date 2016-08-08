#Vigenere Cipher solver
def find_key(ciphertext):
	match=False
	keylength=0
	key=[]
	#this loop will find the length of the key, and the first letter?
	while(match==False):  #increment keylength until find match
		occurrences =[0]*26 #array to store occurences of each letter, mapped based on ascii values, a=97, so offset by 97
		keylength+=1
		print "\nTrying keylength = ",keylength
		
		for i in xrange(0, len(ciphertext), keylength ):#iterate over the cipher text at intervals of keylength
			#print ciphertext[i],ord(ciphertext[i])-97
			occurrences[ord(ciphertext[i])-97]+=1 #number of occurrences of each letter
		print "Letter frequencies:"
		for i in xrange(0, len(occurrences)):#convert to percentages
			occurrences[i] = (occurrences[i]/(float(len(ciphertext))/keylength))*100
			print "{0}: {1}%".format(chr(i+97), occurrences[i])
		for i in xrange(0,26): #verify frequencies against heuristics
			#e ~ 12%, t~9, a~8% :leave 1% margin
			#check e, t, a
			if (occurrences[i]>11 and occurrences[(i+15)%26]>8 and occurrences[(i+22)%26]>7 ): 
				match=True
				print"Found matching frequencies at keylength of",keylength
				#store first offset/first letter in key
				key.append((i+26-4)%26)# e is 4 ahead of a
				print "The first letter of the key is:",chr(key[0]+97)
				break
		
	#we need to loop over the now known length of the key to find the other letters
	for i in xrange(1, keylength):
		occurrences =[0]*26 #array to store occurences of each letter, mapped based on ascii values, a=97, so offset by 97
		for j in xrange(i, len(ciphertext), keylength ):#iterate over the cipher text at intervals of keylength
			occurrences[ord(ciphertext[j])-97]+=1 #frequency as % of occurrences of each letter
		for j in xrange(0, len(occurrences)):#convert to percentages
			occurrences[j] = (occurrences[j]/(float(len(ciphertext))/keylength))*100
		for j in xrange(0,26): #verify frequencies against heuristics
			#e ~ 12%, t~9, a~8% :leave 1% margin
			#check e, t, a
			if (occurrences[j]>11 and occurrences[(j+15)%26]>8 and occurrences[(j+22)%26]>7 ): 
				key.append((j+26-4)%26)# e is 4 ahead of a
				print "The next letter of key is",chr(key[i]+97)
				break
				
	plainkey=""
	for i,s in enumerate(key):
		plainkey+=chr(s+97)
	print "the key is:",plainkey
	return key

def codec (key, input_text, encode=True): #encodes or decodes
	if (encode):
		filename = "cipher_text.txt"
		print "writing encoded message to file:",filename
	else:
		filename = "plaintext.txt"
		print "writing decoded message to file:",filename
		
	output_text=""
	for i,s in enumerate(input_text):
		if (encode):
			output_text+=chr(((ord(s)-97+key[i%len(key)])%26)+97)
		else:
			output_text+=chr(((ord(s)-97-key[i%len(key)])%26)+97)
	with open(filename, 'w') as f:
		f.write(output_text)

if __name__ == "__main__":
	print "vigenere cipher decoder/encoder"
	
	with open('ciphertext.txt', 'r') as f:
		ciphertext = f.read()
	
	key=find_key(ciphertext)
	codec(key,ciphertext,encode=False) #decode
	with open('plaintext.txt', 'r') as f:
		plaintext = f.read()
	codec(key, plaintext,encode=True)