#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"
#include "ssd1306.h"
#include "pillcounter.h"

volatile long pill_weight = 0;
volatile long offset = 0; 

float n_pills = 0;
long hx711_value = 0;
ssd1306_t display;

void bluetooth_setup(){
    uart_init(UART, 9600); // Iniciar UART1 com BaudRate de 9600
    gpio_set_function(HC06_RX,GPIO_FUNC_UART); // Iniciar Pino RX na funçao UART
    gpio_set_function(HC06_TX,GPIO_FUNC_UART); // Iniciar Pino TX na funçao UART
    uart_set_fifo_enabled(UART,true); // Habilitar FIFO para evitar sobrecarga
}

void ssd1306_setup(){
    // Iniciar protocolo I2C
    i2c_init(I2C, 400 * 1000); // Iniciar I2C a uma frequencia de 400bits/sec (FAST MODE)
    gpio_set_function(SSD1306_SCL,GPIO_FUNC_I2C);
    gpio_set_function(SSD1306_SDA,GPIO_FUNC_I2C);
    gpio_pull_up(SSD1306_SCL);
    gpio_pull_up(SSD1306_SDA);

    //Iniciar Display
    ssd1306_init(&display, 128, 64, SSD1306_ADDR, I2C); //Pinos 14 e 15 = i2c1
    ssd1306_clear(&display);
}

static void button_callback(uint gpio, uint32_t events){
    long actual_weight;
    actual_weight = read_hx711();
    if(gpio == BUTTON_A_PIN){
        pill_weight = actual_weight - offset;
    }
    else{
        offset = actual_weight;
    }
}

void buttonA_setup() {
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);
    gpio_set_irq_enabled_with_callback(BUTTON_A_PIN, GPIO_IRQ_EDGE_FALL, true, &button_callback);
}

void buttonB_setup() {
    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_B_PIN);
    gpio_set_irq_enabled_with_callback(BUTTON_B_PIN, GPIO_IRQ_EDGE_FALL, true, &button_callback);
}

void hx711_setup(){
    gpio_init(HX711_DATA_PIN);
    gpio_set_dir(HX711_DATA_PIN, GPIO_IN);

    gpio_init(HX711_SCK_PIN);
    gpio_set_dir(HX711_SCK_PIN, GPIO_OUT);
    gpio_put(HX711_SCK_PIN, 0);
}

long read_hx711(){
    long value = 0;
    
    for(int i=0; i < 24; i++){
        gpio_put(HX711_SCK_PIN,1);
        sleep_us(1);
        value = (value << 1 | gpio_get(HX711_DATA_PIN));
        gpio_put(HX711_SCK_PIN,0);
        sleep_us(1);
    }
    //1 Pulso extra ganho de 128x
    gpio_put(HX711_SCK_PIN, 1);
    sleep_us(1);
    gpio_put(HX711_SCK_PIN, 0);
    sleep_us(1);

    //Converter 24 bits em 32 
    if (value & 0x800000){ //verifica se o mais significativo de value é 1
        value |= 0xFF000000; 
    }
    printf("Valor lido do HX711: %ld\n",value);

    return value;
}

void write_ssd1306(float num_pills){
    ssd1306_clear(&display);
    
    char message[32];
    sprintf(message, "Pilulas: %.2f", num_pills);

    ssd1306_draw_string(&display,0,0,1,message);
    ssd1306_show(&display);
}

void send_bluetooth_mesage(float num_pills){
    char message[32];
    sprintf(message, "Numero de pilulas: %.2f\n", num_pills);
    uart_puts(UART, message);
}

float calculate_pills(){
    float value;
    value = (float)(hx711_value-offset)/pill_weight;
    return value;
}


int main()
{
    stdio_init_all();
    bluetooth_setup();
    ssd1306_setup();
    buttonA_setup();
    buttonB_setup();
    hx711_setup();

    sleep_ms(2000);  // Espera estabilizar

    while (true) {
        hx711_value = read_hx711();
        n_pills = calculate_pills();
        write_ssd1306(n_pills);
        send_bluetooth_mesage(n_pills);

        printf("Peso da pilula: %ld\n", pill_weight);
        printf("Numero de pilulas: %.4f\n", n_pills);
        printf("Valor útil HX711: %ld\n", hx711_value - offset);
        printf("Peso Offset: %ld\n", offset);
        sleep_ms(1000);
    }
}
