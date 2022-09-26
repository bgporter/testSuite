#! /usr/bin/env python3

'''
   makeTest.py -- tiny utility to create a skeleton JUCE UnitTest class.
   Copyright (c) 2018 Brett g Porter

   Given the command line param 'ClassName', will create a file
   `test_ClassName.cpp` either in :
      - a directory named `test` underneath the current directory.
      - in the current directory, if we're already in a directory named `test`.
   It will also attempt to open a file `className.cpp` and append the necessary
   code to include the code in the newly created test file. 
   
   **NOTE** that the macro `RUN_UNIT_TESTS` must be defined and equal to 1 
   in the source file that wants to be tested.    
   
'''

import os

kTemplate = """

#include <juce_core/juce_core.h>

class Test_{0} : public TestSuite
{{
public:
    Test_{0}() 
    : TestSuite("{0}", "!!! category !!!")
    {{

    }}

    void runTest() override
    {{
        beginTest("!!! WRITE SOME TESTS FOR THE {0} Class !!!");

        /*
          To create a test, call `test("testName", testLambda);`
          To (temporarily) skip a test, call `skipTest("testName", testLambda);`
          To define setup for a block of tests, call `setup(setupLambda);`
          To define cleanup for a block of tests, call `tearDown(tearDownLambda);`

          Setup and TearDown lambdas will be called before/after each test that 
          is executed, and remain in effect until explicitly replaced. 

          All the functionality of the JUCE `UnitTest` class is available from
          within these tests. 
        */
    }}
private:
    // !!! test class member vars here...
}};

static Test_{0} test{0};
"""

kCppTemplate = """
#if RUN_UNIT_TESTS
#include "test/test_{0}.cpp"
#endif
"""


def CreateTestFile(className):
    pth = os.getcwd()
    head, currentDir = os.path.split(pth)
    if currentDir != 'test':
        # We're not already in the test directory.
        try:
            pth = os.path.join(pth, "test")
            print ("Creating dir {}".format(pth))
            os.mkdir(pth)
        except OSError:
            # the dir already exists, ignore.
            pass
    fileName = "test_{}.cpp".format(className)
    fullPath = os.path.join(pth, fileName)
    if (not os.path.exists(fullPath)):
       print ("Creating file {}".format(fullPath))
       with open(fullPath, "wt") as f:
           f.write(kTemplate.format(className))
       return True
    else:
       print ("test file already exists -- not creating it.")
       return False

def AddTestInclude(className):
    ''' attempt to open the cpp source file being tested and append the 
        chunk of code that conditionally includes the unit tests.
    '''

    def FindSource(path, className):
        '''
            Look for a file named either "ClassName.cpp" or "className.cpp" in the 
            directory specified by 'path' and return its full path. 

            Will raise FileNotFoundError if appropriate. 
        '''
        fileName = className[0].upper() + className[1:] + ".cpp"
        fullPath = os.path.join(pth, fileName)
        if os.path.exists(fullPath):
            return fullPath

        # try again with leading lowercase:
        fileName = className[0].lower() + className[1:] + ".cpp"
        fullPath = os.path.join(pth, fileName)
        if os.path.exists(fullPath):
            return fullPath

        raise FileNotFoundError

    pth = os.getcwd()
    head, currentDir = os.path.split(pth)
    if currentDir == 'test':
        # we are in the test directory already, need to look above where we are.
        pth = head

    try:
        fullPath = FindSource(pth, className)
        print ("Adding include of the unit tests.")
        with open(fullPath, "at") as f:
            f.write(kCppTemplate.format(className))
        return True
    except FileNotFoundError:
        print ("ERROR: source for {} not found".format(className))
        return False

def Usage():
    print (""" makeTest.py <ClassName>

    1. Creates a unit test file for the specified class in a directory 
       named "test" underneath the directory containing that class file
       (unless the file already exists).
    2. Inserts a block of code at the end of the specified class to include
       the contents of the unit test file inside the cpp file. 

    ASSUMES that the class "FooClass" is contained in a file that's named
    `fooClass.cpp`
    
    """)


if __name__ == "__main__":
    import sys
    try:
        className = sys.argv[1]
    except IndexError:
        className = "--help"

    if "--help" == className:
        Usage()
        sys.exit(0)
        
    if CreateTestFile(className):
       AddTestInclude(className)
