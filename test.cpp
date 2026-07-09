#include <iostream>
#include <string>
int main() {
    std::string s = "test";
    try {
        std::string sub = s.substr(0, 800);
        std::cout << "SUCCESS: " << sub << std::endl;
    } catch(std::exception& e) {
        std::cout << "THREW: " << e.what() << std::endl;
    }
    return 0;
}
