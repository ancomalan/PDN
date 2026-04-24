from mrjob.job import MRJob
import re 

WORD_RE = re.compile(r"[a-zA-Z]+") # regex for letters a-z or A-Z 

# class extends MRJob parent class
class MRLongestWords(MRJob):
    
    # key is first letter of word, value is word itself    
    # go through each line and grab each word 
    def mapper(self, _, line):
        for word in WORD_RE.findall(line):
            first_letter = word[0].lower()
            yield(first_letter, word.lower())

    # all words beginning with same letter are grouped together by key automatically  

    # ex: "a", [alan, apple]
    # get largest word starting with certain letter
    def reducer(self, first_letter, words):
        largest_word_length = 0
        words = list(words) # first loop consumes iterator so change to list
        # get largest length 
        for word in words:
            if len(word) > largest_word_length:
                largest_word_length = len(word)   

        # set comprehension for creating set that contains longest words (no duplicates) 
        longest_words = {word for word in words  if len(word) == largest_word_length} 
        
        yield(first_letter, list(longest_words))


if __name__ == '__main__':
    MRLongestWords.run()