#include "wokwi-api.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
  uint32_t bpmAttr;
  uint32_t spo2Attr;
  uint8_t reg;
  bool gotReg;
} chip_state_t;

static bool on_i2c_connect(void *user_data, uint32_t address, bool read) {
  chip_state_t *chip = (chip_state_t *)user_data;
  if (!read) {
    chip->gotReg = false;
  }
  return true;
}

static bool on_i2c_write(void *user_data, uint8_t data) {
  chip_state_t *chip = (chip_state_t *)user_data;
  if (!chip->gotReg) {
    chip->reg = data;
    chip->gotReg = true;
  }
  return true;
}

static uint8_t on_i2c_read(void *user_data) {
  chip_state_t *chip = (chip_state_t *)user_data;

  if (chip->reg == 0x00) {
    return (uint8_t)attr_read(chip->bpmAttr);
  }

  if (chip->reg == 0x01) {
    return (uint8_t)attr_read(chip->spo2Attr);
  }

  return 0;
}

static void on_i2c_disconnect(void *user_data) {
  (void)user_data;
}

void chip_init() {
  chip_state_t *chip = malloc(sizeof(chip_state_t));
  chip->bpmAttr = attr_init("bpm", 78);
  chip->spo2Attr = attr_init("spo2", 98);
  chip->reg = 0x00;
  chip->gotReg = false;

  i2c_config_t config = {
    .address = 0x57,
    .scl = pin_init("SCL", INPUT_PULLUP),
    .sda = pin_init("SDA", INPUT_PULLUP),
    .connect = on_i2c_connect,
    .read = on_i2c_read,
    .write = on_i2c_write,
    .disconnect = on_i2c_disconnect,
    .user_data = chip,
  };

  i2c_init(&config);
}