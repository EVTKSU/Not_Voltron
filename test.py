class Onboarding:
    def __init__(self, number, word):
        self.number = number
        self.word = word
        
    def write(self):
        print(self.number)
        print(self.word)
            
test = Onboarding(2024, "Voltron")

test.write()
