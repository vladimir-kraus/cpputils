# CppUtils

A collection of C++ utilities.

LeakCheck
---------
File: [`leakcheck.h`](qtutils/leakcheck.h) [`leakcheck.cpp`](qtutils/leakcheck.cpp)<br>
Dependency: none<br>
License: MIT

A set of tools for simple leak checking in your code by tracking instances of classes marked with macro `LEAKCHECK(YourClassName)` and reporting number of instances which have not been deleted at the exit of your program.

In order to check leaks, make the following 3 changes to your code.

1) Leak-checker is switched on by defining macro `LEAKCHECK_ENABLED`. This will easily allow you switching leak checking ON and OFF.

2) Add macro `LEAKCHECK(YourClassName)` to a private section of all your classes whose count of instances you want to track. For example:
```cpp
class Widget
{
    LEAKCHECK(Widget)
public:
    // some code here ...
};
```

3) Move all your code from `main()` function to `do_main()` (or use a different name) and make the following form of `main()`:
```cpp
int main(int argc, char *argv[])
{
    int result = do_main(argc, argv);
#if defined(LEAKCHECK_ENABLED)
    LeakCheck::print_leaks();
    if (result == 0)
    {
        return LeakCheck::has_leaks();
    }
#endif
    return result;
}
```

This will print the report of all leaked instances. For example if you use this code:
```cpp
int do_main(int argc, char *argv[])
{
    auto w = new Widget();
    // some code here ...
    return 0;
    // oops, we forgot to delete the widget
}
```
The program will print the following report:
```
Leak-checker leaks (type, count):
class Widget, 1
```
There are two obvious drawbacks of this tool. Firstly it does not catch leaks of objects which do not have ```LEAKCHECK``` macro in their class definition. This is a matter of fact and it cannot be overcome. And secondly, it does not report at which place in code the leaked object was instantiated. All it shows, is the type of the object. However this is not a big problem in practice. If you develop in small incremental steps and you test often, it is easy to have leak-checking switched ON and you can quickly spot which change in your program introduced the leak. On the other hand, this is much more complicated when you come a to large and leaking code base and you want to find the leaks. In that case it is better to use some other leak checking tools.

Singleton
---------
File: [`singleton.h`](qtutils/singleton.h)<br>
Dependency: none<br>
License: MIT

Yes, I know that singletons are evil. Everybody says that. Nevertheless I must confess that I like to use singletons time to time because they are so pragmatic, helpful and can save lots of code. The only thing you need to remember when using singletons is to use them sparsely and only when you have a very good reason. There are many possible implementation of singletons but after trying several of them I created my own which you can find in [`singleton.h`](qtutils/singleton.h). Like many other singleton implementations also this uses [CTRP pattern](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern). Unlike many other implementations, which are implicitly instantiated at first use, my implementation requires explicit instantiation. Which I think is a good thing because I like to have control over the time when things are instantiated and destroyed. Unlike many other implementations in which the instances live until the very end of your program, this implementation allows destroying the singleton instances and then instantiating them again. This singleton implementation is not dependent on Qt so it can be used in any C++ code. But it complements with all Qt classes extremely well.

Here I would like to mention why I think that singletons are so cool (sometimes). The main benefit of singletons is that they are accessible from anywhere, you do not need to pass and keep their references all around your application. Yes, there is certain price to pay. You hide code dependencies (well, this not cool, in general), testing may be a bit more complicated (but is possible), you must not forget to instantiate them and instantiate them only once (if the instatiation is not automatic).

For example I am using singleton for keeping my application's preferences. There are so many small objects throughout my application which need to get some information from preferences (e.g. about customized data formatting) that it is not practical to pass explicitly reference to preferences to each of these little objects individually. My `Preferences` class use multiple inheritance from `QObject` and `Singleton<Preferences>` (CRTP pattern). Inheriting from `QObject` allows defining for example signal `changed()` to inform the rest of the application about the fact that something in the preferences was changed and let all observers connected to this signal to be updated and reflect these changes. Inheriting from `Singleton<>` provides static `instance()` method and asserts that it is instantiated at most once (in debug mode only).

Note that this singleton implementation is not thread-safe. Thread-safety, if required, must be implemented for the derived class (e.g. by arming the getters and setters in derived `Preferences` class with mutexes). However due to the fact that you have full control over when the singleton is created, make sure you always create is before other thread can access it via `instance()` method. This is a basic prerequisity for correct code.

A snippet from `preferences.h` (with implementations from `preferences.cpp`):
```cpp
#include "singleton.h"
// ...

class Preferences : public QObject, public Singleton<Preferences> // note that Singleton is mentioned only here
{
    Q_OBJECT
    
public:
    Preferences()
    {
        load(); // loaded via QSettings
    }
    
    ~Preferences() override
    {
        save(); // saved via QSettings
    }
    
    // ... some public setters and getters, e.g.
    void setDateFormat(const QString &format)
    {
        m_dateFormat = format;
        emit changed();
    }
    
signals:
    void changed();
    
private:
    // ... some private members, e.g.
    QString m_dateFormat;
};
```

A snippet from any place in the code, e.g. `datatable.cpp` which needs to be updated whenever the preferences change:
```cpp
#include "preferences.h"
// ...

DataTable::DataTable(QWidget *parent) : 
    QWidget(parent)
{
    // ... constructor code
    connect(Preferences::instance(), &Preferences::changed, this, &DataTable::update);
}

// ...
```

A snippet from `main.cpp`:
```cpp
// ...

int main(int argc, char *argv[]) 
{
    QApplication application(argc, argv);
    // ... setup application name etc. so that we can use QSettings
    Preferences preferences; // preferences are instantiated and loaded
    MainWindow window;
    window.show();
    return application.exec();
    // preferences are saved and destroyed
}
```

Another usecase for a singleton in my application is a window which may have only one instance `preferenceswindow.h`:
```cpp
#include "singleton.h"
// ...

class PreferencesWindow : public QWidget, public Singleton<PreferencesWindow> // note that Singleton is mentioned only here
{
// ... 
};
```

And this is how the window is displayed from `mainwindow.cpp`:
```cpp
#include "preferenceswindow.h"
// ...

void MainWindow::openPreferencesWindow()
{
    if (PreferencesWindow::instance() == nullptr)
    {
        auto wnd = new PreferencesWindow(); // it is a top level window so it has no parent
        wnd->setAttribute(Qt::WA_DeleteOnClose); // to be deleted when it is closed by user
    }
    else
    {
        PreferencesWindow::instance()->activateWindow(); // it may have been hidden behind the main window
    }
}
// ...
```
