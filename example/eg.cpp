#include <iostream>

void say_hello_to(std::string name) {
    std::cout << "Hello, " << name << "\n";
    return;
}

int main() {
    std::string student_name{"Ryan"};
    say_hello_to(student_name);
    return 0;
}