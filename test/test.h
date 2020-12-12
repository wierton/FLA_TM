#ifndef TEST_H
#define TEST_H

#define TEST(name)              \
  namespace test {              \
  class test##name {            \
  public:                       \
    test##name();               \
  };                            \
  }                             \
  static test::test##name name; \
                                \
  test::test##name::test##name()

#endif
