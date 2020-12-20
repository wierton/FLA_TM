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

bool validate(const std::string &s) {
  int a1 = 0, b1 = 0, a2 = 0, b2 = 0;

  int i = 0;

  for (i = 0; i < s.size(); i++)
    if (s[i] == 'a')
      a1++;
    else
      break;
  for (; i < s.size(); i++)
    if (s[i] == 'b')
      b1++;
    else
      break;
  for (; i < s.size(); i++)
    if (s[i] == 'a')
      a2++;
    else
      break;
  for (; i < s.size(); i++)
    if (s[i] == 'b')
      b2++;
    else
      break;

  if (a1 == 0 || a2 == 0 || b1 == 0 || b2 == 0 ||
      a1 != a2 || b1 != b2)
    return false;

  if (i < s.size()) return false;
  return true;
}

TEST(case1) {
  for (int i = 0; i < 10000; i++) {
    int a1 = rand() % 5;
    int b1 = rand() % 5;
    int a2 = rand() % 5;
    int b2 = rand() % 5;
    std::string t;
    for (int i = 0; i < a1; i++) t.push_back('a');
    for (int i = 0; i < b1; i++) t.push_back('b');
    for (int i = 0; i < a2; i++) t.push_back('a');
    for (int i = 0; i < b2; i++) t.push_back('b');

    std::ifstream ifs("programs/case1.tm");
    TMParser parser;
    auto TM = parser.parseTMFile(ifs);
    TM.set_input(t);
    std::string result = TM.run();

    if (a1 == 0 || a2 == 0 || b1 == 0 || b2 == 0 ||
        a1 != a2 || b1 != b2) {
      if (result != "false")
        std::cout << "ans <> false, fail at " << t << "\n";
    } else {
      if (result != "true")
        std::cout << "ans <> true, fail at " << t << "\n";
    }
  }
}

TEST(case2) {
  for (int i = 0; i < 100000; i++) {
    std::string t;
    for (int i = rand() % 10; i >= 0; i --)
      switch (rand() % 2) {
      case 0: t.push_back('a'); break;
      case 1: t.push_back('b'); break;
      }

    std::ifstream ifs("programs/case1.tm");
    TMParser parser;
    auto TM = parser.parseTMFile(ifs);
    TM.set_input(t);
    std::string result = TM.run();

    if (!validate(t)) {
      if (result != "false")
        std::cout << "ans <> false, fail at " << t << "\n";
    } else {
      if (result != "true")
        std::cout << "ans <> true, fail at " << t << "\n";
    }
  }
}
