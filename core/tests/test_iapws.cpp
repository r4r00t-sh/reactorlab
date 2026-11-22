#include "reactorlab/iapws/if97_region1.hpp"

#include <gtest/gtest.h>

TEST(IapwsIf97, Region1Table5Sample) {
  // IAPWS R7-97 Table 5: p=3 MPa, T=300 K
  const auto prop = reactorlab::iapws::region1(3.0e6, 300.0);
  EXPECT_NEAR(prop.v, 1.00215168e-3, 1.0e-11);
  EXPECT_NEAR(prop.rho, 1.0 / 1.00215168e-3, 1.0e-5);
}

TEST(IapwsIf97, Region1PwrLike) {
  const auto prop = reactorlab::iapws::region1(15.5e6, 580.0);
  EXPECT_GT(prop.rho, 650.0);
  EXPECT_LT(prop.rho, 800.0);
  EXPECT_GT(prop.cp, 4000.0);
  EXPECT_LT(prop.cp, 8000.0);
}
