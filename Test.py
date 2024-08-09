import os

class Test: 
    def __init__(self):
        self.test = None
        self.home = os.getcwd()

    def change_test(self):
        self.test = "colin"

        print(self.test)

    def get_directory(self):
        print(self.home)    

if __name__== '__main__':
    t = Test()

    t.change_test()
    t.get_directory()