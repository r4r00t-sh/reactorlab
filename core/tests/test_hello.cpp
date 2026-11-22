#include "reactorlab/hello.hpp"
#include "reactorlab/version.hpp"

#include <gtest/gtest.h>
#include <string>

TEST(HelloWorld, VersionIsNonEmpty) {
  EXPECT_FALSE(reactorlab::version_string().empty());
  EXPECT_EQ(reactorlab::version_string(), "0.7.0");
}

TEST(HelloWorld, GreetingContainsVersionAndDisclaimer) {
  const std::string msg = reactorlab::hello("Nanma");
  EXPECT_NE(msg.find("Nanma"), std::string::npos);
  EXPECT_NE(msg.find("0.7.0"), std::string::npos);
  EXPECT_NE(msg.find("educational/research"), std::string::npos);
}

TEST(HelloWorld, DisclaimerMentionsNotSafetyGrade) {
  const auto disclaimer = reactorlab::educational_disclaimer();
  EXPECT_NE(std::string(disclaimer).find("not licensed"), std::string::npos);
  EXPECT_NE(std::string(disclaimer).find("safety-grade"), std::string::npos);
}
