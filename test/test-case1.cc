#include <cassert>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <utility>
#include <vector>

#include "test.h"

#include "../main.cc"

TEST(case1) {
  for (int i = 0; i < 100000; i ++) {
    int a1 = rand() % 5;
    int b1 = rand() % 5;
    int a2 = rand() % 5;
    int b2 = rand() % 5;
    std::string t;
    for (int i = 0; i < a1; i ++)
      t.push_back('a');
    for (int i = 0; i < b1; i ++)
      t.push_back('b');
    for (int i = 0; i < a2; i ++)
      t.push_back('a');
    for (int i = 0; i < b2; i ++)
      t.push_back('b');

    std::ifstream ifs("programs/case1.tm");
    TMParser parser;
    auto TM = parser.parseTMFile(ifs);
    TM.set_input(t);
    std::string result = TM.run();

    if (a1 == 0 || a2 == 0 || b1 == 0 || b2 == 0
        || a1 != a2 || b1 != b2) {
      if (result != "false")
        std::cout << "ans <> false, fail at " << t << "\n";
    } else {
      if (result != "true")
        std::cout << "ans <> true, fail at " << t << "\n";
    }
  }
}
