#include <cstdint>
#include <iostream>
#include <vector>

constexpr static size_t kCnt = 6;

int main() {
    std::vector<uint64_t> ans {179};
    ans.push_back(1);
    for (size_t i = 1; i < kCnt; ++i) {
        uint64_t x = ans.back();
        uint64_t mul = i;
        mul *= i+1;
        mul /= 2;
        ans.push_back(x * mul);
    }
    for (size_t i = 1; i < ans.size(); ++i) {
        std::cout << i << " -> " << ans[i] << "\n";
    }
}