/**
 * @file pwm.c
 * @brief PWM driver implementation
 *
 * Implements functions for controlling PWM outputs using the ESP32 LEDC
 * peripheral.
 */

#include "pwm.h"

#include "driver/ledc.h"
#include "esp_log.h"

// PWM用のタグ定義
static const char *TAG = "PWM";

// PWM設定用の定数
#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES LEDC_TIMER_13_BIT  // 0-8191の解像度
#define LEDC_FREQUENCY 5000              // 5kHz
#define LEDC_MAX_CHANNELS 8              // ESP32は8チャンネルまでサポート

// PWM用の構造体
typedef struct {
  ledc_channel_t channel;
  gpio_num_t gpio;
  bool in_use;
} pwm_channel_t;

static pwm_channel_t pwm_channels[LEDC_MAX_CHANNELS] = {0};
static bool pwm_initialized = false;

/**
 * @brief PWM機能の初期化
 *
 * LEDC周波数とタイマーを設定します
 *
 * @return kSuccess 初期化成功時
 */
fn_t drv_pwm_init(void) {
  if (pwm_initialized) {
    return kSuccess;
  }

  // LEDCのタイマー設定
  ledc_timer_config_t ledc_timer = {.speed_mode = LEDC_MODE,
                                    .timer_num = LEDC_TIMER,
                                    .duty_resolution = LEDC_DUTY_RES,
                                    .freq_hz = LEDC_FREQUENCY,
                                    .clk_cfg = LEDC_AUTO_CLK};

  // タイマー設定を適用
  esp_err_t err = ledc_timer_config(&ledc_timer);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "PWM timer setup failed with error: %d", err);
    return kFailure;
  }

  // チャンネル構造体を初期化
  for (int i = 0; i < LEDC_MAX_CHANNELS; i++) {
    pwm_channels[i].in_use = false;
    pwm_channels[i].channel = i;
  }

  pwm_initialized = true;
  ESP_LOGI(TAG, "PWM initialized successfully");
  return kSuccess;
}

/**
 * @brief 指定したGPIOピンをPWM出力として設定
 *
 * @param gpio_pin GPIOピン番号
 * @param initial_duty 初期デューティ値 (0-100%)
 * @return 成功時にチャンネル番号、失敗時に-1
 */
int drv_pwm_setup_pin(gpio_num_t gpio_pin, uint8_t initial_duty) {
  ESP_LOGI(TAG, "Setting up PWM on GPIO %d with duty %d%%", gpio_pin,
           initial_duty);

  if (!pwm_initialized) {
    ESP_LOGI(TAG, "PWM not initialized, initializing now");
    if (drv_pwm_init() != kSuccess) {
      ESP_LOGE(TAG, "PWM initialization failed");
      return -1;
    }
  }

  // 空きチャンネルを探す
  int channel_idx = -1;
  for (int i = 0; i < LEDC_MAX_CHANNELS; i++) {
    if (!pwm_channels[i].in_use) {
      channel_idx = i;
      break;
    }
  }

  if (channel_idx == -1) {
    ESP_LOGE(TAG, "No free PWM channels available");
    return -1;  // 空きチャンネルがない
  }

  // ピンのリセットと設定
  gpio_reset_pin(gpio_pin);
  esp_err_t gpio_err = gpio_set_direction(gpio_pin, GPIO_MODE_OUTPUT);
  if (gpio_err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set GPIO direction: %d", gpio_err);
    return -1;
  }

  // 0-100%を0-8191の範囲に変換
  uint32_t duty = (initial_duty * 8191) / 100;

  // チャンネル設定
  ledc_channel_config_t ledc_channel = {
      .channel = pwm_channels[channel_idx].channel,
      .duty = duty,
      .gpio_num = gpio_pin,
      .speed_mode = LEDC_MODE,
      .hpoint = 0,
      .timer_sel = LEDC_TIMER};

  esp_err_t err = ledc_channel_config(&ledc_channel);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "PWM channel config failed with error: %d", err);
    return -1;
  }

  // チャンネル情報を保存
  pwm_channels[channel_idx].in_use = true;
  pwm_channels[channel_idx].gpio = gpio_pin;

  ESP_LOGI(TAG, "PWM setup successful on channel %d", channel_idx);
  return channel_idx;
}

/**
 * @brief PWMのデューティ比を設定
 *
 * @param channel チャンネル番号
 * @param duty デューティ比 (0-100%)
 * @return kSuccess 成功時、kFailure 失敗時
 */
fn_t drv_pwm_set_duty(int channel, uint8_t duty) {
  if (channel < 0 || channel >= LEDC_MAX_CHANNELS ||
      !pwm_channels[channel].in_use) {
    ESP_LOGE(TAG, "Invalid PWM channel: %d", channel);
    return kFailure;
  }

  // 0-100%を0-8191の範囲に変換
  uint32_t duty_scaled = (duty * 8191) / 100;

  esp_err_t err =
      ledc_set_duty(LEDC_MODE, pwm_channels[channel].channel, duty_scaled);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "PWM set duty failed with error: %d", err);
    return kFailure;
  }

  err = ledc_update_duty(LEDC_MODE, pwm_channels[channel].channel);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "PWM update duty failed with error: %d", err);
    return kFailure;
  }

  ESP_LOGI(TAG, "PWM duty set to %d%% on channel %d", duty, channel);
  return kSuccess;
}

/**
 * @brief PWMチャンネルを無効化
 *
 * @param channel チャンネル番号
 * @return kSuccess 成功時、kFailure 失敗時
 */
fn_t drv_pwm_disable(int channel) {
  if (channel < 0 || channel >= LEDC_MAX_CHANNELS ||
      !pwm_channels[channel].in_use) {
    return kFailure;
  }

  esp_err_t err = ledc_stop(LEDC_MODE, pwm_channels[channel].channel, 0);
  if (err != ESP_OK) {
    return kFailure;
  }

  pwm_channels[channel].in_use = false;
  return kSuccess;
}