//
// Copyright (c) Vladimir Kraus. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for details.
//

// A set of tools for simple leak checking in your code by tracking
// instances of classes marked with macro LEAKCHECK(YourClassName)
// and reporting number of instances which have not been deleted
// at the exit of your program.

//
// Usage:
//
// In order to check leaks, make the following 3 changes to your code.
//
// 1) Leak-checker is switched on by defining macro LEAKCHECK_ENABLED.
//    This will easily allow you switching leak checking ON and OFF.
//
// 2) Add macro LEAKCHECK(YourClassName) to a private section of all your
//    classes whose count of instances you want to track.
//
// 3) Move all your code from main() function to do_main() (or use
//    a different name) and make the following form of main():
//
// int main(int argc, char *argv[])
// {
//     int result = do_main(argc, argv);
//
// #if defined(LEAKCHECK_ENABLED)
//     LeakCheck::print_leaks();
//     if (result == 0)
//     {
//         return LeakCheck::has_leaks();
//     }
// #endif
//
//     return result;
// }

#pragma once

#if defined(LEAKCHECK_ENABLED)

#include <map>
#include <mutex>
#include <string>

class LeakCheck
{
public:
    /**
     * @brief Prints a table of all cleases whose instances have leaked and number of leaks.
     *        This function should be called immediately before your program exits.
     */
    static void print_leaks();

    /**
     * @brief Prints a table of number of existing instances, total number of instances
     *        and maximum number of instances of each class recorded during program run.
     *        This function can be called at any time during your program run.
     */
    static void print_stats();

    /**
     * @brief Check whether any class instances have leaked.
     *        This function should be called immediately before your program exits.
     * @return @c true if any undeleted object is found, @c false otherwise
     */
    static bool has_leaks();

    /**
     * @brief Returns the current number of instances of class T.
     */
    template <typename T>
    static long instance_count()
    {
        const std::lock_guard<std::mutex> lock(s_mutex);
        return s_stats[std::string(typeid(T).name())].count;
    }

protected:
    static void update(const char *typeName, int increment);

private:
    struct Entry
    {
        long count = 0;
        long max = 0;
        long total = 0;
    };

    static std::mutex s_mutex;
    static std::map<std::string, Entry> s_stats;
};

template<typename T>
class LeakChecker : public LeakCheck
{
public:
    LeakChecker()
    {
        update(1);
    }

    LeakChecker(const LeakChecker &/*other*/)
    {
        update(1);
    }

    ~LeakChecker()
    {
        update(-1);
    }

private:
    void update(int increment)
    {
        LeakCheck::update(typeid(T).name(), increment);
    }
};

/**
  * @brief Mark the classes whose instances you want to track with this macro.
  *        Add this macro to the private section of the class.
  *        Example of Widget class:
  *
  *        class Widget
  *        {
  *            LEAKCHECK(Widget)
  *        public:
  *        ...
  *        }
  */
#define LEAKCHECK(Type) LeakChecker<Type> __leakCheck;

#else

/**
  * @brief This macro is empty unless LEAKCHECK_ENABLED macro is defined.
  */
#define LEAKCHECK(Type)

#endif
