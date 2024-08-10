#include "raft_addr.h"

#include <gtest/gtest.h>

#include <iostream>

#include "simple_random.h"

TEST(RaftAddr, RaftAddr) {
  vraft::RaftAddr addr;
  bool b = addr.FromString("127.0.0.1:9988#7");
  ASSERT_EQ(b, true);
  std::cout << addr.ToString() << std::endl;

  ASSERT_EQ(addr.port(), 9988);
  ASSERT_EQ(addr.id(), 7);
  ASSERT_EQ(addr.ToString(), std::string("127.0.0.1:9988#7"));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}