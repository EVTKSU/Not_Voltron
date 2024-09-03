class myclass:
    def __init__ (self, x, y):
        self.x = x
        self.y = y
    def display_info(self):
        print(f"variable 1: {self.x}, variable 2: {self.y}")

    @classmethod
    def class_method (cls):
        print("ay")

def main():
    instance = myclass("ex", 42)
    instance.display_info()
    myclass.class_method()

if __name__ == '__main__':
    main()