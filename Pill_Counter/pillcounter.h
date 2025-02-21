#ifndef PILLCOUNTER_H
#define PILLCOUNTER_H

// Definição de Pinos
#define HC06_RX 9
#define HC06_TX 8
#define SSD1306_SDA 14
#define SSD1306_SCL 15
#define BUTTON_A_PIN 5
#define BUTTON_B_PIN 6
#define HX711_DATA_PIN 2
#define HX711_SCK_PIN 3

// Definição Endereço
#define SSD1306_ADDR 0x3C

// Definição Comunicação
#define UART uart1
#define I2C i2c1

void bluetooth_setup();
void ssd1306_setup();
static void button_callback(uint gpio, uint32_t events);
void buttonA_setup();
void buttonB_setup();
void hx711_setup();
long read_hx711();
void write_ssd1306(float num_pills);
void send_bluetooth_mesage(float num_pills);
float calculate_pills();

#endif