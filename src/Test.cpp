// src/Test.cpp
class Foo {
public:
    Foo() {}
    // missing copy constructor / assignment operator
};

int main() {
    Foo a;
    Foo b = a; // This should trigger -Weffc++
    return 0;
}

//to manually cc with -Weffc++:
//c++ -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Weffc++ -Wold-style-cast -Wconversion -pedantic -std=c++20 -Iinclude src/Server.cpp -o Test

