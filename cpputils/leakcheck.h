#pragma once

#if defined(LEAKCHECK_ENABLED)

#include <map>
#include <mutex>
#include <string>

class LeakChecker
{
public:
    static void print_leaks();
    static void print_stats();
    static bool has_leaks();

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
class LeakCheck : public LeakChecker
{
public:
    LeakCheck()
    {
        update(1);
    }

    LeakCheck(const LeakCheck &/*other*/)
    {
        update(1);
    }

    ~LeakCheck()
    {
        update(-1);
    }

private:
    void update(int increment)
    {
        LeakChecker::update(typeid(T).name(), increment);
    }
};

#define LEAK_CHECK(Type) LeakCheck<Type> __leakCheck;

#else

#define LEAK_CHECK(Type)

#endif
