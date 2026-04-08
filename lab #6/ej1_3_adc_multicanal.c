/* ============================================================
 * Práctica 6 - Ejercicios 1, 2 y 3
 * ADC Multicanal: Potenciómetro, LDR, LM35
 * STM32F103C8 | STM32CubeIDE | HAL
 * ============================================================
 *
 * CONFIGURACIÓN EN CUBEMX (hacer ANTES de usar este código):
 * ----------------------------------------------------------
 * ADC1:
 *   - IN0 → PA0 (Potenciómetro)
 *   - IN1 → PA1 (LDR)
 *   - IN2 → PA2 (LM35 / sensor temperatura)
 *   - Scan Mode: ENABLE
 *   - Continuous Conversion: DISABLE (por ahora)
 *   - Number Of Conversion: 3
 *   - Rank 1 → Channel 0, SampleTime 55.5 Cycles
 *   - Rank 2 → Channel 1, SampleTime 55.5 Cycles
 *   - Rank 3 → Channel 2, SampleTime 55.5 Cycles
 *   - Data Alignment: Right
 *   - External Trigger: Software
 *
 * CONEXIONES:
 *   PA0 → cursor del potenciómetro (extremos a GND y 3.3V)
 *   PA1 → divisor resistivo con LDR (LDR entre 3.3V y PA1,
 *          resistencia 10kΩ entre PA1 y GND)
 *   PA2 → salida del LM35 (Vout = 10mV/°C)
 * ============================================================ */

#include "main.h"
#include "stdio.h"   // para sprintf
#include "string.h"  // para strlen (LCD)

/* -- Handles generados por CubeMX -- */
extern ADC_HandleTypeDef hadc1;
// extern UART_HandleTypeDef huart1; // opcional para debug

/* ============================================================
 * EJERCICIO 1 — Lectura raw de 3 canales ADC (sin DMA)
 * Lee cada canal de forma secuencial con polling.
 * ============================================================ */
void Ejercicio1_LeerADC_Multicanal(void)
{
    uint16_t adc_pot  = 0;   // Canal 0: Potenciómetro
    uint16_t adc_ldr  = 0;   // Canal 1: LDR
    uint16_t adc_temp = 0;   // Canal 2: LM35

    /* --- Canal 0: Potenciómetro --- */
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    adc_pot = HAL_ADC_GetValue(&hadc1);

    /* --- Canal 1: LDR --- */
    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    adc_ldr = HAL_ADC_GetValue(&hadc1);

    /* --- Canal 2: LM35 --- */
    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    adc_temp = HAL_ADC_GetValue(&hadc1);

    HAL_ADC_Stop(&hadc1);

    /* Guardar para uso posterior */
    (void)adc_pot;
    (void)adc_ldr;
    (void)adc_temp;
}

/* ============================================================
 * EJERCICIO 2 — Conversión a magnitudes físicas
 *
 *  Voltaje (V)     : V    = (adc / 4095.0) * 3.3
 *  Temperatura(°C) : T    = V_lm35 * 100   (LM35: 10mV/°C)
 *  Iluminación (%) : luz  = (adc_ldr / 4095.0) * 100
 *  Potenciómetro(%): pot  = (adc_pot / 4095.0) * 100
 * ============================================================ */

/* Estructura para guardar todos los valores procesados */
typedef struct {
    uint16_t raw_pot;
    uint16_t raw_ldr;
    uint16_t raw_temp;

    float voltaje_pot;   // Voltios
    float voltaje_ldr;   // Voltios
    float voltaje_temp;  // Voltios

    float temperatura;   // °C  (LM35)
    float iluminacion;   // %   (LDR)
    float posicion_pot;  // %   (Potenciómetro)
} SensoresData_t;

/* Convierte los valores raw del ADC a magnitudes físicas */
void Ejercicio2_ConvertirMagnitudes(SensoresData_t *datos,
                                    uint16_t raw_pot,
                                    uint16_t raw_ldr,
                                    uint16_t raw_temp)
{
    const float VREF      = 3.3f;
    const float ADC_MAX   = 4095.0f;

    datos->raw_pot  = raw_pot;
    datos->raw_ldr  = raw_ldr;
    datos->raw_temp = raw_temp;

    /* Voltajes */
    datos->voltaje_pot  = (raw_pot  / ADC_MAX) * VREF;
    datos->voltaje_ldr  = (raw_ldr  / ADC_MAX) * VREF;
    datos->voltaje_temp = (raw_temp / ADC_MAX) * VREF;

    /* Temperatura: LM35 entrega 10mV por °C */
    datos->temperatura = datos->voltaje_temp * 100.0f;

    /* Iluminación relativa: 0% = oscuro, 100% = máxima luz
     * NOTA: depende del divisor resistivo.
     * Si LDR está entre 3.3V y PA1, mayor luz = mayor voltaje */
    datos->iluminacion  = (raw_ldr  / ADC_MAX) * 100.0f;

    /* Posición del potenciómetro en % */
    datos->posicion_pot = (raw_pot  / ADC_MAX) * 100.0f;
}

/* ============================================================
 * EJERCICIO 3 — Visualización en LCD
 * Utiliza las funciones LCD de práctica anterior.
 * Se asume que tienes: LCD_SetCursor(col, row) y LCD_Print(str)
 * ============================================================ */

/* Declaraciones externas de tu driver LCD */
extern void LCD_SetCursor(uint8_t col, uint8_t row);
extern void LCD_Print(char *str);
extern void LCD_Clear(void);

void Ejercicio3_MostrarEnLCD(SensoresData_t *datos)
{
    char linea1[17];  // LCD 16x2 → máx 16 chars + '\0'
    char linea2[17];

    /* --- Fila 0: Temperatura y Luz --- */
    // Formato: "T:25.3C  Luz:67%"
    snprintf(linea1, sizeof(linea1), "T:%4.1fC  L:%3.0f%%",
             datos->temperatura,
             datos->iluminacion);

    /* --- Fila 1: Potenciómetro y Voltaje --- */
    // Formato: "Pot:45%  V:1.48V"
    snprintf(linea2, sizeof(linea2), "Pot:%3.0f%% V:%4.2fV",
             datos->posicion_pot,
             datos->voltaje_pot);

    LCD_SetCursor(0, 0);
    LCD_Print(linea1);

    LCD_SetCursor(0, 1);
    LCD_Print(linea2);
}

/* ============================================================
 * FUNCIÓN PRINCIPAL DE INTEGRACIÓN (Ej 1 + 2 + 3)
 * Llamar dentro del while(1) del main.c
 * ============================================================ */
void Practica6_Ej1_2_3_Loop(void)
{
    uint16_t adc_pot  = 0;
    uint16_t adc_ldr  = 0;
    uint16_t adc_temp = 0;
    SensoresData_t sensores = {0};

    /* 1. Lectura ADC multicanal */
    HAL_ADC_Start(&hadc1);

    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    adc_pot = HAL_ADC_GetValue(&hadc1);

    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    adc_ldr = HAL_ADC_GetValue(&hadc1);

    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    adc_temp = HAL_ADC_GetValue(&hadc1);

    HAL_ADC_Stop(&hadc1);

    /* 2. Conversión a magnitudes físicas */
    Ejercicio2_ConvertirMagnitudes(&sensores, adc_pot, adc_ldr, adc_temp);

    /* 3. Mostrar en LCD */
    Ejercicio3_MostrarEnLCD(&sensores);

    HAL_Delay(500);  // Actualizar cada 500ms
}
