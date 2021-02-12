#include "leakcheck.h"

#if defined(LEAKCHECK_ENABLED)

#include <cassert>
#include <iostream>

std::map<std::string, LeakCheck::Entry> LeakCheck::s_stats;

std::mutex LeakCheck::s_mutex;

void LeakCheck::update(const char *typeName, int increment)
{
    assert(increment == 1 || increment == -1);

    const std::lock_guard<std::mutex> lock(s_mutex);

    Entry &e = s_stats[std::string(typeName)];

    e.count += increment;
    assert(e.count >= 0);

    if (increment > 0)
    {
        e.total += increment;
    }

    if (e.count > e.max)
    {
        e.max = e.count;
    }
}

void LeakCheck::print_leaks()
{
    const std::lock_guard<std::mutex> lock(s_mutex);

    std::cout << "Leak-checker leaks (type, count):" << std::endl;
    int leaks = 0;

    for (const auto & kv : s_stats)
    {
        if (kv.second.count > 0)
        {
            std::cout << kv.first << ", " << kv.second.count << std::endl;
            leaks += kv.second.count;
        }
    }

    if (leaks == 0)
    {
        std::cout << "no leaks" << std::endl;
    }
    else
    {
        std::cout << leaks << " leaks found" << std::endl;
    }

    std::cout << std::endl;
}

void LeakCheck::print_stats()
{
    const std::lock_guard<std::mutex> lock(s_mutex);

    std::cout << "Leak-checker stats (type, count, total, max):" << std::endl;

    for (const auto & kv : s_stats)
    {
        std::cout << kv.first << ", " << kv.second.count << ", " << kv.second.total << ", " << kv.second.max << std::endl;
    }

    std::cout << std::endl;
}

bool LeakCheck::has_leaks()
{
    for (const auto & kv : s_stats)
    {
        if (kv.second.count > 0)
        {
            return true;
        }
    }

    return false;
}

#endif
