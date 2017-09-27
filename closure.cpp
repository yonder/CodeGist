/**
 * @file closure.cpp
 * @author yingshin(izualzhy@163.com)
 * @date 2015/11/30 15:57:41
 * @version $Revision$ 
 * @brief 
 *  
 **/

#include <iostream>
#include <string>

class Closure {
public:
    Closure() {}
    ~Closure() {}

    virtual void Run() = 0;
};//Closure

class FunctionClosure0 : public Closure {
public:
    typedef void (*FunctionType)();

    FunctionClosure0(FunctionType f) :
        _f(f) {
        }
    ~FunctionClosure0() {
    }

    virtual void Run() {
        _f();
        delete this;
    }
private:
    FunctionType _f;
};//FunctionClosure0

template <typename Class>
class MethodClosure0 : public Closure {
public:
    typedef void (Class::*MethodType)();

    MethodClosure0(Class* object, MethodType m) :
        _object(object),
        _m(m) {
        }
    ~MethodClosure0() {
    }

    virtual void Run() {
        std::cout << "address of object in Run : " << _object << std::endl;
        (_object->*_m)();
        delete this;
    }
private:
    Class* _object;
    MethodType _m;
};//MethodClosure0

template <typename Arg1>
class FunctionClosure1 : public Closure {
public:
    typedef void (*FunctionType)(Arg1);

    FunctionClosure1(FunctionType f, Arg1 arg1) :
        _f(f),
        _arg1(arg1) {
        }
    ~FunctionClosure1() {
    }

    virtual void Run() {
        _f(_arg1);
        delete this;
    }
private:
    FunctionType _f;
    Arg1 _arg1;
};//FunctionClosure1

template <typename Class, typename Arg1>
class MethodClosure1 : public Closure {
public:
    typedef void (Class::*MethodType)(Arg1);

    MethodClosure1(Class* object, MethodType m, Arg1 arg1) :
        _object(object),
        _m(m),
        _arg1(arg1) {
        }
    ~MethodClosure1() {
    }

    virtual void Run() {
        std::cout << "address of object in Run : " << _object << std::endl;
        (_object->*_m)(_arg1);
        delete this;
    }
private:
    Class* _object;
    MethodType _m;
    Arg1 _arg1;
};//MethodClosure1

Closure* NewCallback(void (*function)()) {
    return new FunctionClosure0(function);
}

template <typename Class>
Closure* NewCallback(Class* object, void (Class::*method)()) {
    return new MethodClosure0<Class>(object, method);
}

template <typename Arg1>
Closure* NewCallback(void(*function)(Arg1), Arg1 arg1) {
    return new FunctionClosure1<Arg1>(function, arg1);
}

template <typename Class, typename Arg1>
Closure* NewCallback(Class* object, void (Class::*method)(Arg1), Arg1 arg1) {
    return new MethodClosure1<Class, Arg1>(object, method, arg1);
}

void foo() {
    std::cout << "foo" << std::endl;
}

template<typename type>
void foo(type data) {
    std::cout << "foo data=" << data << std::endl;
}

class Foo {
public:
    void foo() {
        std::cout << "Foo::foo" << std::endl;
    }

    template<typename type>
    void foo1(type data) {
        std::cout << "Foo::foo data=" << data << std::endl;
    }
};//Foo

int main()
{
    Foo f;
    Closure* closure;
    std::cout << "address of object in main : " << &f << std::endl;

    closure = NewCallback(foo);
    closure->Run();

    closure = NewCallback(&f, &Foo::foo);
    closure->Run();

    closure = NewCallback(foo<const char [7]>, "string");
    closure->Run();

    closure = NewCallback(&f, &Foo::foo1<double>, 100.0);
    closure->Run();

    return 0;
}

/* vim: set ts=4 sw=4 sts=4 tw=100 */