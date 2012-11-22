#include "gtest/gtest.h"
#include "config.h"

TEST(WeightedRandomChooser, uniform_ab) {
  double weights[] = {1, 1};
  double weights_sum = 2;

  WeightedRandomChooser wrc;
  const uint TRIALS = 100000;
  uint results[2] = {0};

  for(uint i=0; i<TRIALS; i++) {
    int idx = wrc.choose(2, weights, weights_sum);
    results[idx]++;
  }

  EXPECT_EQ(TRIALS, results[0]+results[1]);

  //wolfram:  CDF[BinomialDistribution[100000, 0.5], 49000] = 1.2e-10
  EXPECT_GT(results[0], 49000);
  EXPECT_LT(results[0], 51000);
}

TEST(WeightedRandomChooser, uniform_20) {
  double weights[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  double weights_sum = 20;
  EXPECT_EQ(20, sizeof(weights)/sizeof(weights[0]));

  WeightedRandomChooser wrc;
  const uint TRIALS = 1000000;
  uint results[20] = {0};
  for(uint i=0; i<20; i++) { results[i] = 0; }

  for(uint i=0; i<TRIALS; i++) {
    int idx = wrc.choose(20, weights, weights_sum);
    results[idx]++;
  }

  int sum = 0;
  for(uint i=0; i<20; i++) {
      sum += results[i];
  }
  EXPECT_EQ(TRIALS, sum);

  for(uint i=0; i<20; i++) {
      //wolfram:  CDF[BinomialDistribution[1000000, 0.05], 49000] = 2.1e-6
      EXPECT_GT(results[i], 49000);
      EXPECT_LT(results[i], 51000);
  }
}


TEST(WeightedRandomChooser, skewed_90_10_ab) {
  double weights[] = {9, 1};
  double weights_sum = 10;

  WeightedRandomChooser wrc;
  const uint TRIALS = 100000;
  uint results[2] = {0};

  for(uint i=0; i<TRIALS; i++) {
    int idx = wrc.choose(2, weights, weights_sum);
    results[idx]++;
  }

  EXPECT_EQ(TRIALS, results[0]+results[1]);

  //wolfram:  CDF[BinomialDistribution[100000, 0.9], 89500] = 8.5e-8
  //wolfram:  1 - CDF[BinomialDistribution[100000, 0.9], 90500] = 5.3e-8
  EXPECT_GT(results[0], 89500);
  EXPECT_LT(results[0], 90500);
}
