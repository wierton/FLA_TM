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
  int a = 0;
  int b = 0;
  int res = 0;

  int i = 0;
  for (i = 0; i < s.size(); i++)
    if (s[i] == '1')
      a++;
    else
      break;

  // std::cout << s.substr(i) << "\n";
  if (a == 0 || s[i] != 'x') return false;

  for (i++; i < s.size(); i++)
    if (s[i] == '1')
      b++;
    else
      break;

  // std::cout << s.substr(i) << "\n";
  if (b == 0 || s[i] != '=') return false;

  for (i++; i < s.size(); i++)
    if (s[i] == '1')
      res++;
    else
      break;

  // std::cout << a << ", " << b << ", " << res << "\n";
  if (res == 0 || i < s.size()) return false;
  return a * b == res;
}

TEST(case2_1) {
  for (int a = 1; a < 100; a++) {
    for (int b = 1; b < 100; b++) {

      std::string t;
      for (int i = 0; i < a; i++) t.push_back('1');
      t.push_back('x');
      for (int i = 0; i < b; i++) t.push_back('1');
      t.push_back('=');
      for (int i = 0; i < a * b; i++) t.push_back('1');

      std::ifstream ifs("programs/case2.tm");
      TMParser parser;
      auto TM = parser.parseTMFile(ifs);
      TM.set_input(t);
      std::string result = TM.run();

      assert(validate(t));
      if (result != "true") {
        std::cout << "ans <> true, fail at " << t << "\n";
      }
    }
  }
}

TEST(case2_2) {
  for (int a = 1; a < 10; a++) {
    for (int b = 1; b < 10; b++) {
      for (int res = 1; res < 200; res++) {

        std::string t;
        for (int i = 0; i < a; i++) t.push_back('1');
        t.push_back('x');
        for (int i = 0; i < b; i++) t.push_back('1');
        t.push_back('=');
        for (int i = 0; i < res; i++) t.push_back('1');

        std::ifstream ifs("programs/case2.tm");
        TMParser parser;
        auto TM = parser.parseTMFile(ifs);
        TM.set_input(t);
        std::string result = TM.run();

        if (a * b == res) {
          assert (validate(t));
          if (result != "true") {
            std::cout << "ans <> true, fail at " << t
                      << "\n";
          }
        } else {
          assert (!validate(t));
          if (result != "false") {
            std::cout << "ans <> false, fail at " << t
                      << "\n";
          }
        }
      }
    }
  }
}

TEST(case2_3) {
  for (int i = 0; i < 100000; i++) {
    std::string t;
    for (int i = rand() % 20; i > 0; i--) {
      switch (rand() % 5) {
      case 0: t.push_back('x'); break;
      case 1: t.push_back('='); break;
      default: t.push_back('1'); break;
      }
    }

    std::ifstream ifs("programs/case2.tm");
    TMParser parser;
    auto TM = parser.parseTMFile(ifs);
    TM.set_input(t);
    std::string result = TM.run();

    if (validate(t)) {
      if (result != "true") {
        std::cout << "ans <> true, fail at " << t << "\n";
      }
    } else {
      if (result != "false") {
        std::cout << "ans <> false, fail at " << t << "\n";
      }
    }
  }
}
