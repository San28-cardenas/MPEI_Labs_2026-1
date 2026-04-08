/* ============================================================
 * Práctica 6 - Ejercicios 4 y 5
 * ADC Modo Continuo + DMA Circular
 * STM32F103C8 | STM32CubeIDE | HAL
 * ============================================================
 *
 * CONFIGURACIÓN EN CUBEMX (cambios respecto a Ej 1-3):
 * ----------------------------------------------------------
 * ADC1:
 *   - Scan Mode:               ENABLE
 *   - Continuous Conversion:   ENABLE       ← NUEVO (Ej 4)
 *   - Number Of Conversion:    3
 *   - Rank 1 → Channel 0 (PA0), 55.5 Cycles
 *   - Rank 2 → Channel 1 (PA1), 55.5 Cycles
 *   - Rank 3 → Channel 2 (PA2), 55.5 Cycles
 *   - Data Alignment:          Right
 *   - DMA Continuous Requests: ENABLE       ← NUEVO (Ej 5)
 *
 * DMA (pestaña DMA Settings dentro de ADC1):
 *   - Agregar canal DMA para ADC1
 *   - Direction:   Peripheral To Memory
 *   - Mode:        Circular               ← IMPORTANTE
 *   - Data Width:  Half Word (16 bits) en ambos lados
 *   - Increment:   Memory increment ON, Peripheral OFF
 *
 * RESULTADO: El buffer adc_buffer[] se actualiza
 * automáticamente SIN intervención del CPU.
 * ============================================================ */

#include "main.h"
#include <string.h>

/* -- Handles generados por CubeMX -- */
extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

/* ============================================================
 * BUFFER DMA — debe ser global/estático
 * [0] = Potenciómetro  (PA0)
 * [1] = LDR            (PA1)
 * [2] = LM35/Temp      (PA2)
 * ============================================================ */
#define ADC_NUM_CANALES  3

volatile uint16_t adc_buffer[ADC_NUM_CANALES] = {0};

/* Flag que indica cuando el DMA completó un ciclo (opcional) */
volatile uint8_t  dma_conversion_completa = 0;

/* ============================================================
 * EJERCICIO 4 — ADC en modo continuo (sin DMA)
 *
 * En modo continuo el ADC repite las conversiones de forma
 * automática. Solo se inicia una vez y los valores se
 * actualizan solos. El CPU aún debe leer manualmente.
 * ============================================================ */
void Ejercicio4_ADC_ModoContinuo_Init(void)
{
    /* Iniciar conversión continua — solo se llama UNA VEZ */
    HAL_ADC_Start(&hadc1);
    /* A partir de aquí el ADC convierte continuamente.
     * En el loop principal se puede leer con PollForConversion
     * pero ya no hay que re-iniciar la conversión */
}

/* Lectura en modo continuo dentro del while(1) */
void Ejercicio4_ADC_ModoContinuo_Leer(uint16_t *pot,
                                       uint16_t *ldr,
                                       uint16_t *temp)
{
    /* Con modo continuo + Scan, el ADC cicla los 3 canales
     * automáticamente. PollForConversion espera que el canal
     * actual termine. */
    HAL_ADC_PollForConversion(&hadc1, 10);  // timeout 10ms
    *pot = HAL_ADC_GetValue(&hadc1);

    HAL_ADC_PollForConversion(&hadc1, 10);
    *ldr = HAL_ADC_GetValue(&hadc1);

    HAL_ADC_PollForConversion(&hadc1, 10);
    *temp = HAL_ADC_GetValue(&hadc1);
    /* NOTA: Con modo continuo el CPU sigue "atado" a esperar
     * la conversión. El Ejercicio 5 con DMA elimina esto. */
}

/* ============================================================
 * EJERCICIO 5 — ADC + DMA Circular
 *
 * Con DMA circular:
 *  - El ADC convierte los 3 canales continuamente
 *  - El DMA copia cada resultado a adc_buffer[] sin CPU
 *  - El programa principal solo lee adc_buffer[]
 *  - El CPU queda LIBRE para otras tareas
 * ============================================================ */

/* Iniciar ADC con DMA — llamar UNA SOLA VEZ en main() */
void Ejercicio5_ADC_DMA_Init(void)
{
    /*
     * HAL_ADC_Start_DMA inicia:
     *   1. El ADC en modo continuo + scan
     *   2. El DMA en modo circular
     *   3. La transferencia automática al buffer
     *
     * Parámetros:
     *   &hadc1            → handle del ADC
     *   (uint32_t*)adc_buffer → destino en memoria
     *   ADC_NUM_CANALES   → cantidad de conversiones por ciclo
     */
    HAL_ADC_Start_DMA(&hadc1,
                      (uint32_t*)adc_buffer,
                      ADC_NUM_CANALES);
}

/* ============================================================
 * CALLBACK — se ejecuta automáticamente cuando el DMA
 * completa un ciclo completo (los 3 canales).
 * Definir en main.c o en este archivo.
 * ============================================================ */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1)
    {
        /* El buffer ya fue actualizado por el DMA.
         * Señalamos al programa principal que hay datos nuevos */
        dma_conversion_completa = 1;

        /* Con DMA circular NO es necesario reiniciar nada.
         * El DMA vuelve al inicio del buffer automáticamente. */
    }
}

/* ============================================================
 * LECTURA DEL BUFFER (para usar en el while(1))
 *
 * El programa principal simplemente lee adc_buffer[].
 * No hay espera, no hay polling → CPU completamente libre.
 * ============================================================ */
void Ejercicio5_Leer_Buffer(uint16_t *pot,
                             uint16_t *ldr,
                             uint16_t *temp)
{
    /* Lectura atómica: deshabilitar interrupciones
     * brevemente para evitar lectura parcial */
    __disable_irq();
    *pot  = adc_buffer[0];
    *ldr  = adc_buffer[1];
    *temp = adc_buffer[2];
    __enable_irq();
}

/* ============================================================
 * VERIFICACIÓN — comprueba que el DMA está funcionando
 *
 * Uso: llamar en el while(1), si retorna 1 los datos
 * se están actualizando automáticamente.
 * ============================================================ */
uint8_t Ejercicio5_Verificar_DMA_Activo(void)
{
    if (dma_conversion_completa)
    {
        dma_conversion_completa = 0;  // limpiar flag
        return 1;  // DMA activo y funcionando
    }
    return 0;
}

/* ============================================================
 * EJEMPLO DE USO EN main.c
 * ============================================================
 *
 * int main(void)
 * {
 *     // ... inicializaciones de CubeMX ...
 *
 *     // Iniciar DMA — UNA SOLA VEZ
 *     Ejercicio5_ADC_DMA_Init();
 *
 *     while (1)
 *     {
 *         uint16_t pot, ldr, temp;
 *
 *         // Leer buffer (sin bloquear el CPU)
 *         Ejercicio5_Leer_Buffer(&pot, &ldr, &temp);
 *
 *         // Procesar y mostrar...
 *         HAL_Delay(200);
 *     }
 * }
 * ============================================================ */
