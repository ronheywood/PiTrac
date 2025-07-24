/*
 * Camera Bounded Context - Test Main Entry Point
 * 
 * Boost Test framework main for Camera bounded context unit tests.
 */

#define BOOST_TEST_MODULE CameraBoundedContextTests

// Support both header-only and linked Boost Test modes
#ifdef BOOST_TEST_HEADER_ONLY
    #include <boost/test/included/unit_test.hpp>
#else
    #include <boost/test/unit_test.hpp>
#endif

// Disable memory leak detection for debug builds to avoid OpenCV false positives
#ifdef _WIN32
    #ifdef _DEBUG
        #include <crtdbg.h>
    #endif
#endif

// Global test setup and teardown can be added here if needed
struct GlobalTestFixture {
    GlobalTestFixture() {
        // Global test initialization
        BOOST_TEST_MESSAGE("Starting Camera Bounded Context Tests");
        
        // Disable memory leak detection on Windows debug builds
        // This prevents false positives from OpenCV's internal allocations
        #ifdef _WIN32
            #ifdef _DEBUG
                _CrtSetDbgFlag(0);  // Disable memory leak detection
            #endif
        #endif
    }
    
    ~GlobalTestFixture() {
        // Global test cleanup
        BOOST_TEST_MESSAGE("Completed Camera Bounded Context Tests");
    }
};

BOOST_GLOBAL_FIXTURE(GlobalTestFixture);
