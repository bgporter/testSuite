//
// Copyright (c) 2022 Brett g Porter. All Rights Reserved.
//

#pragma once

/*
BEGIN_JUCE_MODULE_DECLARATION
ID:               testSuite
vendor:           bgporter
version:          1.0.0
name:             TestSuite
description:      An extension to JUCE's UnitTest system.
website:          https://github.com/bgporter/testSuite
license:          MIT
dependencies:     juce_core
END_JUCE_MODULE_DECLARATION
 */

#include <juce_core/juce_core.h>

/**
 * @class TestSuite
 * @brief An extension to the JUCE UnitTest system.
 */

class TestSuite : public juce::UnitTest
{
  public:
    TestSuite(const juce::String &name, const juce::String &category = juce::String()) : UnitTest(name, category)
    {
    }

    /**
     * Pass in a function to perform common setup required by multiple
     * tests.
     *
     * Called automatically before each call of the `test()` method, remains
     * in effect until a new setup function is passed to the test class.
     *
     * If your test class needs resources that are allocated once for
     * all of your subtests, you can handle that by overriding the
     * `UnitTest::initialise()` method.
     *
     * Default does nothing.
     */
    void setup(std::function<void(void)> setupFn)
    {
        onSetup = setupFn;
    }

    /**
     * Pass in a function to perform any common cleanup needed by your subtests.
     *
     * If your test class allocated resources in the `initialise()` method that
     * stayed in scope for all of your subtests, you should handle that cleanup
     * by overriding the `UnitTest::shutdown()` method.
     *
     * If the class you're testing has a method that lets you check a class
     * invariant, adding a call inside the `TearDown()` method like:
     * ```
     *    // call your class invariant checker
     *    expect (this->IsValid());
     * ```
     *
     * Lets you check that each test not only succeeded on its own terms, but
     * left the object being tested in a valid state.
     */
    void tearDown(std::function<void(void)> tearDownFn)
    {
        onTearDown = tearDownFn;
    }

    /**
     * The Test method:
     * * performs common setup.
     * * executes a single subtest
     * * performs the common cleanup.
     *
     * Your unit tests will be a sequence of calls to `test(...)` inside of your
     * test suite class' `runTest()` method.
     *
     * @param testName Name of the sub-test.
     * @param testFn   Test function to call.
     * @sa SkipTest
     */
    void test(juce::StringRef testName, std::function<void(void)> testFn)
    {
        beginTest(testName);
        onSetup();
        testFn();
        onTearDown();
    }

    /**
     * An easy way to disable a test that's implemented by calling an std::function
     * that doesn't just comment (or conditionally compile) out that test. Your
     * test log will include a line helping you remember that the test wasn't run.
     *
     * Assumption is that you may encounter working tests that momentarily break
     * so we want to temporarily disable them. Instead of commenting or `#ifdef`-ing
     * those tests out, change the function call from `test(...)` to
     * `skipTest(...)`. To re-enable the test, just change that method call back
     * to `test(...)`.
     *
     * @param testName name/description of this test.
     * @param testFn   test function to (not) call.
     *
     * @sa Test
     */
    void skipTest(juce::StringRef testName, std::function<void(void)> /*testFn*/)
    {
        logMessage("-----------------------------------------------------------------");
        logMessage(TRANS("WARNING: Skipping ") + getName() + " / " + testName);
    }

    /// default setup/tearDown do nothing.
    inline static std::function<void(void)> noOp{[]() {}};

    inline static const juce::String QuitAfterTests{"--quitAfterTests"};
    inline static const juce::String DisableTests{"--disableTests"};
    inline static const juce::String EnableTests{"--enableTests"};
    inline static const juce::String AssertOnFail{"--assertOnFail"};
    inline static const juce::String ContinueOnFail{"--continueOnFail"};
    inline static const juce::String LogPasses{"--logPasses"};
    inline static const juce::String RandomSeed{"--randomTestSeed"};

    /**
     * @brief Load and run all of the unit tests defined in this project.
     *
     * Meant to be called from the top of your app's `initialise()`
     * method and passed the command line that's provided to your
     * application.
     *
     * May be completely disabled by setting the macro RUN_UNIT_TESTS
     * to zero.
     *
     * DEFAULT behaviors:
     * in all cases:
     *     - only failures are logged (unless cmdline flag `--logPasses`)
     *     - random seed for tests is 0 (unless cmdline flag `--randomTestSeed <<intVal>>`)
     *     - returns true to continue unless cmdline flag `--quitAfterTests`)
     *
     * in debug builds (JUCE_DEBUG is set):
     *     - all tests are run (unless cmdline flag `--disableTests`)
     *     - first test fail asserts (unless cmdline flag `--continueOnFail`)
     *
     * in release builds (JUCE_DEBUG is not set):
     *     - no tests run (unless cmdline flag `--enableTests`)
     *     - always continue after test failure (unless cmdline flag `--assertOnFail`)
     *
     * e.g.:
     *
     *          void MyApplication::initialise (const String& commandLine)
     *          {
     *               if (! TestSuite::runAllTests (commandLine))
     *                   quit ();
     *               // ...rest of initialise code...
     *          }
     *
     * @param commandLine   as defined above
     * @return true         to tell application to continue normally
     * @return false        to tell application it should exit immediately.
     */

    static bool runAllTests(const juce::String &commandLine)
    {
        bool continueRunning{true};
#if RUN_UNIT_TESTS
        bool runTests{false};
        juce::StringArray commands{juce::StringArray::fromTokens(commandLine, true)};
        continueRunning = (-1 == commands.indexOf(TestSuite::QuitAfterTests, true));
        bool logPasses{-1 != commands.indexOf(TestSuite::LogPasses, true)};
        bool assertOnFail{true};
        juce::int64 randomSeed{0};
        if (auto seedIndex{commands.indexOf(TestSuite::RandomSeed, true)}; (seedIndex > -1))
        {
            // will return an empty string if the index is out of bounds...
            auto &seedValue{commands[seedIndex + 1]};
            randomSeed = seedValue.getLargeIntValue();
        }

#if JUCE_DEBUG
        // in debug builds, run tests UNLESS command line turns them off.
        runTests = (-1 == commands.indexOf(TestSuite::DisableTests, true));
        // in debug builds, assert on fail unless the continue flag is present
        assertOnFail = (-1 == commands.indexOf(TestSuite::ContinueOnFail, true));
#else
        // in release builds, don't run tests unless explicitly run.
        runTests = (-1 != commands.indexOf(TestSuite::EnableTests, true));
        // in release builds, continue on test faiure unless the assert flag is set.
        assertOnFail = (-1 != commands.indexOf(TestSuite::AssertOnFail, true));
#endif

        if (runTests)
        {
            juce::UnitTestRunner testRunner;
            testRunner.setAssertOnFailure(assertOnFail);
            testRunner.setPassesAreLogged(logPasses);
            testRunner.runAllTests(randomSeed);
        }
#endif
        return continueRunning;
    }

  private:
    /// function to be called before each test.
    std::function<void(void)> onSetup{noOp};
    /// function to be called after each test.
    std::function<void(void)> onTearDown{noOp};
};
