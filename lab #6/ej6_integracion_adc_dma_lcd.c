/* ============================================================
 * Práctica 6 - Ejercicio 6
 * Integración completa: ADC + DMA + LCD
 * STM32F103C8 | STM32CubeIDE | HAL
 * ============================================================
 *
 * Este es el archivo PRINCIPAL que integra todo.
 * Reemplaza el contenido de tu main.c con esto, o copia
 * las secciones que correspondan.
 *
 * REQUIERE que en CubeMX tengas configurado:
 *   - ADC1: Scan + Continuo + DMA (ver Ej 4-5)
 *   - LCD:  I2C o pines GPIO (según práctica anterior)
 *   - Timer para el delay no bloqueante (opcional)
 * ============================================================ */

#include "main.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>   // abs()

/* ============================================================
 * HANDLES (generados por CubeMX)
 * ============================================================ */
extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

/* LCD — ajusta según tu driver */
extern void LCD_Init(void);
extern void LCD_SetCursor(uint8_t col, uint8_t row);
extern void LCD_Print(char *str);
extern void LCD_Clear(void);

/* ============================================================
 * BUFFER DMA — global, volatile
 * ============================================================ */
#define ADC_CANALES  3

volatile uint16_t adc_dma_buf[ADC_CANALES] = {0};
volatile uint8_t  datos_listos = 0;

/* ============================================================
 * ESTRUCTURA DE DATOS PROCESADOS
 * ============================================================ */
typedef struct {
    /* Raw ADC */
    uint16_t raw[ADC_CANALES];
    /* Magnitudes físicas */
    float temperatura;    // °C
    float iluminacion;    // %
    float potenciometro;  // %
    float voltaje_pot;    // V
} DatosSensores_t;

static DatosSensores_t sensores = {0};

/* ============================================================
 * ANTI-PARPADEO LCD
 *
 * Estrategia: solo reescribir una línea si su contenido
 * cambió respecto a la última vez mostrada.
 * Esto evita el clear() continuo que causa parpadeo.
 * ============================================================ */
static char lcd_linea_anterior[2][17] = {"", ""};

void LCD_PrintSiCambio(uint8_t fila, char *nueva_linea)
{
    if (strncmp(nueva_linea, lcd_linea_anterior[fila], 16) != 0)
    {
        LCD_SetCursor(0, fila);
        LCD_Print(nueva_linea);
        strncpy(lcd_linea_anterior[fila], nueva_linea, 16);
    }
}

/* ============================================================
 * PROCESAMIENTO — convierte raw ADC a magnitudes físicas
 * ============================================================ */
void Procesar_Sensores(DatosSensores_t *d)
{
    const float VREF    = 3.3f;
    const float ADC_MAX = 4095.0f;

    /* Leer buffer de forma atómica */
    __disable_irq();
    d->raw[0] = adc_dma_buf[0];  // Potenciómetro
    d->raw[1] = adc_dma_buf[1];  // LDR
    d->raw[2] = adc_dma_buf[2];  // LM35
    __enable_irq();

    /* Voltaje del potenciómetro */
    d->voltaje_pot    = (d->raw[0] / ADC_MAX) * VREF;

    /* Temperatura: LM35 → 10mV/°C */
    float v_temp      = (d->raw[2] / ADC_MAX) * VREF;
    d->temperatura    = v_temp * 100.0f;

    /* Iluminación relativa (%) */
    d->iluminacion    = (d->raw[1] / ADC_MAX) * 100.0f;

    /* Posición potenciómetro (%) */
    d->potenciometro  = (d->raw[0] / ADC_MAX) * 100.0f;
}

/* ============================================================
 * VISUALIZACIÓN LCD — formato limpio, sin parpadeo
 *
 * Línea 0: "T:25.3C  Luz: 67%"
 * Línea 1: "Pot: 45%  V:1.48V"
 * ============================================================ */
void Mostrar_LCD(DatosSensores_t *d)
{
    char buf0[17];
    char buf1[17];

    snprintf(buf0, sizeof(buf0), "T:%4.1fC L:%3.0f%%",
             d->temperatura,
             d->iluminacion);

    snprintf(buf1, sizeof(buf1), "Pot:%3.0f%% V:%4.2fV",
             d->potenciometro,
             d->voltaje_pot);

    /* Solo actualizar si el contenido cambió */
    LCD_PrintSiCambio(0, buf0);
    LCD_PrintSiCambio(1, buf1);
}

/* ============================================================
 * CALLBACK DMA — se ejecuta al completar cada ciclo
 * ============================================================ */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1)
    {
        datos_listos = 1;
        /* DMA circular → NO reiniciar, continúa solo */
    }
}

/* ============================================================
 * MAIN — estructura completa
 * ============================================================ */

/*
int main(void)
{
    // --- Inicializaciones de CubeMX ---
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_DMA_Init();     // ¡DMA SIEMPRE antes que ADC!
    MX_ADC1_Init();
    MX_I2C1_Init();    // o SPI, según tu LCD

    // --- Inicializar LCD ---
    LCD_Init();
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_Print("Iniciando...");

    HAL_Delay(1000);

    // --- Calibrar ADC (recomendado para F103) ---
    HAL_ADCEx_Calibration_Start(&hadc1);

    // --- Iniciar ADC + DMA circular (una sola vez) ---
    HAL_ADC_Start_DMA(&hadc1,
                      (uint32_t*)adc_dma_buf,
                      ADC_CANALES);

    // --- Limpiar LCD antes del loop ---
    LCD_Clear();

    // --- Loop principal ---
    uint32_t ultimo_update = 0;

    while (1)
    {
        uint32_t ahora = HAL_GetTick();

        // Actualizar LCD cada 300ms sin usar HAL_Delay (no bloqueante)
        if ((ahora - ultimo_update) >= 300)
        {
            ultimo_update = ahora;

            if (datos_listos)
            {
                datos_listos = 0;

                // Procesar señales
                Procesar_Sensores(&sensores);

                // Mostrar en LCD (solo si cambió)
                Mostrar_LCD(&sensores);
            }
        }

        // Aquí puedes agregar otras tareas concurrentes
        // sin que el LCD ni el ADC lo bloqueen
    }
}
*/

/* ============================================================
 * NOTAS IMPORTANTES
 * ============================================================
 *
 * 1. En CubeMX, el MX_DMA_Init() DEBE inicializarse ANTES
 *    que MX_ADC1_Init(). CubeMX lo genera en el orden
 *    correcto si configuras DMA desde la pestaña ADC.
 *
 * 2. HAL_ADCEx_Calibration_Start() mejora la precisión del
 *    ADC en el STM32F103. Siempre recomendado.
 *
 * 3. El anti-parpadeo funciona comparando strings.
 *    Si quieres más precisión, filtra primero (Ej 7).
 *
 * 4. Con DMA circular, NUNCA llames HAL_ADC_Stop_DMA()
 *    en el loop — eso detiene el DMA permanentemente.
 *
 * 5. La variable adc_dma_buf[] DEBE ser volatile para que
 *    el compilador no la optimice fuera del loop.
 * ============================================================ */
