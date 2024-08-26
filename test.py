class Test:
  """Test class 
  """
  
  def __init__(self, property: float = 1.0):
    """Initialization function for Test 

    Args:
      property (float, optional): Test property for the class, Defaults to 1.0
    """
    self.test_property: float = property
  
  
  def do_a_test(self, test_param: int) -> None:
    """Method in Test class 

    Args:
      test_param (int): Testing parameter for the Test method
    """
    
    print(f'Test paramater given as {test_param}')
    
    
  def __repr__(self) -> str:
    return f'Test class with test property of {self.test_property}'
    
  
  
  
def main() -> None:
  """Main code section 
  """
  
  test_obj: Test = Test(0.025)
  
  print('\033[H\033[2J', end='')
  test_obj.do_a_test(12)
  print(f'{test_obj}\n')



if __name__ == '__main__':  
  main()
  
  
# Note to self:
#   - Run conda env export > environment.yml to get the enviroment file
#   - Run pip install -r requiremnts.txt to get the requirements
#   - Run conda update python to update conda enviroment