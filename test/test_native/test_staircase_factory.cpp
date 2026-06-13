#include "fakes/FakePWMDevice.h"
#include <gtest/gtest.h>
#include <list>
#include <map>
#include <vector>

class StaircaseFactoryTest : public ::testing::Test {};

struct StepMapping {
  uint8_t stepId;
  uint8_t expanderI2CAddress;
  uint8_t channelWarm;
  uint8_t channelCold;
};

struct Config {
  std::list<StepMapping> stepMappings;
};

class StairStep {
public:
  explicit StairStep(const StepMapping &stepMapping, IPWMDevice *pwmDevice)
      : stepMapping(stepMapping), pwmDevice(pwmDevice){};

  void setWarm(uint16_t value) {
    pwmDevice->setPWM(stepMapping.channelWarm, value);
  }

private:
  StepMapping stepMapping;
  IPWMDevice *pwmDevice;
};

class StaircaseFactory {
public:
  static std::vector<std::unique_ptr<StairStep>>
  createStaircaseFromConfig(const Config &config, std::map<uint8_t, IPWMDevice *> &pwmDevices) {
    std::vector<std::unique_ptr<StairStep>> staircases;

    for (const auto &stepMapping : config.stepMappings) {
      auto deviceIt = pwmDevices.find(stepMapping.expanderI2CAddress);
      if (deviceIt != pwmDevices.end()) {
        staircases.push_back(std::make_unique<StairStep>(stepMapping, deviceIt->second));
      }
    }

    return staircases;
  }
};

namespace {
constexpr StepMapping stepMapping1{
    .stepId = 1, .expanderI2CAddress = 0x40, .channelWarm = 5, .channelCold = 6};
constexpr StepMapping stepMapping2{
    .stepId = 2, .expanderI2CAddress = 0x40, .channelWarm = 7, .channelCold = 8};

FakePWMDevice fakePWM(16);

std::map<uint8_t, IPWMDevice *> fakeDevices = {{stepMapping1.expanderI2CAddress, &fakePWM}};

std::list<StepMapping> stepMappings = {stepMapping1, stepMapping2};

Config config1 = {.stepMappings = stepMappings};
} // namespace

TEST_F(StaircaseFactoryTest, CreateStaircaseFromConfig) {

  auto staircases = StaircaseFactory::createStaircaseFromConfig(config1, fakeDevices);

  ASSERT_TRUE(staircases.size() == 2);
  staircases.at(0)->setWarm(123);
  staircases.at(1)->setWarm(456);

  EXPECT_EQ(fakePWM.getPWM(stepMapping1.channelWarm), 123);
  EXPECT_EQ(fakePWM.getPWM(stepMapping2.channelWarm), 456);
}