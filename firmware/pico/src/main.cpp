#include <cstdio>
#include <cstring>

#include "Quadruped.h"
#include "hardware/uart.h"
#include "pico/stdlib.h"

#define UART_ID uart1
#define UART_BAUD_RATE 9600

#define UART_TX 4
#define UART_RX 5

// servo pulse limits (µs)
static constexpr uint16_t S_MIN = 560;
static constexpr uint16_t S_MAX = 2600;

// leg geometry
static constexpr float COXA_LEN = 0.0f;
static constexpr float FEMUR_LEN = 10.0f;
static constexpr float TIBIA_LEN = 10.0f;

// GPIO assignments
// JointConfig: { gpio, minPulse, maxPulse, minAngle, maxAngle, offset, inverted }
static const QuadrupedConfig ROBOT_CONF = {
    .frontLeft =
        {
            .coxa = {18, S_MIN, S_MAX, 0.0f, 180.0f, 0.0f, false},
            .femur = {17, S_MIN, S_MAX, 12.0f, 180.0f, 0.0f, true},
            .tibia = {16, S_MIN, S_MAX, 0.0f, 110.0f, 0.0f, false},
            .coxaLength = COXA_LEN,
            .femurLength = FEMUR_LEN,
            .tibiaLength = TIBIA_LEN,
        },
    .rearLeft =
        {
            .coxa = {19, S_MIN, S_MAX, 0.0f, 180.0f, 0.0f, false},
            .femur = {20, S_MIN, S_MAX, 12.0f, 180.0f, 0.0f, true},
            .tibia = {21, S_MIN, S_MAX, 0.0f, 110.0f, 0.0f, false},
            .coxaLength = COXA_LEN,
            .femurLength = FEMUR_LEN,
            .tibiaLength = TIBIA_LEN,
        },
    .frontRight =
        {
            .coxa = {13, S_MIN, S_MAX, 0.0f, 180.0f, -7.2f, false},
            .femur = {14, S_MIN, S_MAX, 12.0f, 180.0f, -7.2f, false},
            .tibia = {15, S_MIN, S_MAX, 0.0f, 110.0f, 0.0f, true},
            .coxaLength = COXA_LEN,
            .femurLength = FEMUR_LEN,
            .tibiaLength = TIBIA_LEN,
        },
    .rearRight =
        {
            .coxa = {12, S_MIN, S_MAX, 0.0f, 180.0f, 0.0f, false},
            .femur = {10, S_MIN, S_MAX, 12.0f, 180.0f, 7.2f, false},
            .tibia = {11, S_MIN, S_MAX, 0.0f, 110.0f, -3.6f, true},
            .coxaLength = COXA_LEN,
            .femurLength = FEMUR_LEN,
            .tibiaLength = TIBIA_LEN,
        },
    .bodyLength = 15.0f,
    .bodyWidth = 12.0f,
};

static void reply(const char* msg) {
  printf("%s", msg);
  uart_puts(UART_ID, msg);
}

using Handler = void (*)(Quadruped&);

struct Command {
  const char* name;
  Handler run;
};

static void setLegs(Quadruped& r, bool on) {
  Leg* legs[] = {&r.rearLeftLeg(), &r.rearRightLeg(), &r.frontLeftLeg(),
                 &r.frontRightLeg()};
  for (auto* leg : legs) on ? leg->enable() : leg->disable();
}

static void readline(char* buf, size_t maxlen) {
  size_t n = 0;
  while (n < maxlen - 1) {
    const int c = getchar();
    if (c == '\n' || c == '\r') break;
    buf[n++] = static_cast<char>(c);
  }
  while (n > 0 && (buf[n - 1] == '\r' || buf[n - 1] == ' ')) --n;
  buf[n] = '\0';
}

static const Command COMMANDS[] = {
    {"home",
     [](Quadruped& r) {
       r.home();
       reply("Homed.\n");
     }},
    {"stand",
     [](Quadruped& r) {
       r.stand();
       reply("Standing.\n");
     }},
    {"walk",
     [](Quadruped& r) {
       reply("Walking...\n");
       r.trot(10);
       reply("Done.\n");
     }},
    {"up",
     [](Quadruped& r) {
       r.setStance(15.0f, 0.0f, 0.0f);
       reply("Looking up.\n");
     }},
    {"down",
     [](Quadruped& r) {
       r.setStance(-15.0f, 0.0f, 0.0f);
       reply("Looking down.\n");
     }},
    {"right",
     [](Quadruped& r) {
       r.setStance(0.0f, 15.0f, 0.0f);
       reply("Tilting right.\n");
     }},
    {"crouch",
     [](Quadruped& r) {
       r.setStance(0.0f, 0.0f, -5.0f);
       reply("Crouching.\n");
     }},
    {"off",
     [](Quadruped& r) {
       setLegs(r, false);
       reply("Powered off.\n");
     }},
    {"on",
     [](Quadruped& r) {
       setLegs(r, true);
       reply("Powered on.\n");
     }},
};

static void dispatch(Quadruped& robot, const char* cmd) {
  for (const auto& c : COMMANDS) {
    if (strcmp(cmd, c.name) == 0) {
      c.run(robot);
      return;
    }
  }
  reply("Unknown command: ");
  reply(cmd);
  reply("\n");
  printCommandList();
}

int main() {
  setvbuf(stdout, NULL, _IONBF, 0);

  stdio_init_all();

  //  initialize UART
  uart_init(UART_ID, UART_BAUD_RATE);

  gpio_set_function(UART_RX, GPIO_FUNC_UART);
  gpio_set_function(UART_TX, GPIO_FUNC_UART);

  uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);

  printf("UART READY\n");

  static Quadruped robot(ROBOT_CONF);

  printf("Initializing...\n");
  uart_puts(UART_ID, "Initializing.\n");

  robot.init();

  char bt_cmd[64];
  size_t bt_index = 0;

  reply("Ready.\n");
  printCommandList();

  while (true) {
    if (!uart_is_readable(UART_ID)) continue;

    const char c = uart_getc(UART_ID);
    if (c == '\r') continue;

    if (c == '\n') {
      bt_cmd[bt_index] = '\0';
      if (bt_index > 0) dispatch(robot, bt_cmd);
      bt_index = 0;
    } else if (bt_index < sizeof(bt_cmd) - 1) {
      bt_cmd[bt_index++] = c;
    }
  }
}
