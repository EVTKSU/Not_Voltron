class test:
    def __init__(self, string):
        self.string = string;
    
    def test(self):
        print(self.string)

def main():
    result = test("this is a test")
    result.test()

if __name__ == "__main__":
    main()