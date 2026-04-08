/* ============================================================
 * Práctica 6 - Ejercicio 7 (Opcional)
 * Filtrado de señales ADC
 * STM32F103C8 | STM32CubeIDE | HAL
 * ============================================================
 *
 * Se implementan dos filtros:
 *   A) Promedio móvil (Moving Average) — ventana de N muestras
 *   B) Filtro recursivo (IIR de primer orden / EMA)
 *
 * Ambos reducen el ruido de las mediciones ADC.
 * El código permite comparar con/sin filtrado en el LCD.
 * ============================================================ */

#include "main.h"
#include <stdint.h>
#include <string.h>

/* ============================================================
 * PARÁMETROS CONFIGURABLES
 * ============================================================ */
#define VENTANA_PROMEDIO   8     // Muestras para promedio móvil
#define ALPHA_EMA          0.1f  // Factor EMA: 0=muy suavizado,
                                 //             1=sin filtro
                                 // Recomendado: 0.05 ~ 0.2

/* ============================================================
 * FILTRO A: PROMEDIO MÓVIL (Moving Average)
 *
 * Promedia las últimas N muestras.
 * Ventajas: elimina bien el ruido aleatorio
 * Desventajas: introduce retardo = N/2 muestras
 * ============================================================ */
typedef struct {
    uint16_t buffer[VENTANA_PROMEDIO];
    uint8_t  indice;
    uint8_t  lleno;
    uint32_t suma;
} FiltroPromedio_t;

/* Inicializar un filtro de promedio */
void FiltroPromedio_Init(FiltroPromedio_t *f)
{
    memset(f->buffer, 0, sizeof(f->buffer));
    f->indice = 0;
    f->lleno  = 0;
    f->suma   = 0;
}

/* Agregar muestra y obtener promedio filtrado */
uint16_t FiltroPromedio_Actualizar(FiltroPromedio_t *f,
                                    uint16_t nueva_muestra)
{
    /* Restar la muestra que va a ser reemplazada */
    f->suma -= f->buffer[f->indice];

    /* Agregar nueva muestra */
    f->buffer[f->indice] = nueva_muestra;
    f->suma += nueva_muestra;

    /* Avanzar índice circular */
    f->indice = (f->indice + 1) % VENTANA_PROMEDIO;

    /* Marcar como lleno después de la primera vuelta */
    if (f->indice == 0) f->lleno = 1;

    /* Calcular promedio */
    uint8_t n = f->lleno ? VENTANA_PROMEDIO : (f->indice == 0 ? VENTANA_PROMEDIO : f->indice);
    return (uint16_t)(f->suma / n);
}

/* ============================================================
 * FILTRO B: FILTRO RECURSIVO EMA
 * (Exponential Moving Average / IIR primer orden)
 *
 * y[n] = alpha * x[n] + (1 - alpha) * y[n-1]
 *
 * Ventajas: muy liviano (1 multiplicación), sin buffer
 * Desventajas: respuesta más lenta que el promedio
 *              ante cambios bruscos reales
 * ============================================================ */
typedef struct {
    float   valor_filtrado;
    uint8_t inicializado;
} FiltroEMA_t;

/* Inicializar filtro EMA */
void FiltroEMA_Init(FiltroEMA_t *f)
{
    f->valor_filtrado = 0.0f;
    f->inicializado   = 0;
}

/* Aplicar filtro EMA y retornar valor filtrado */
uint16_t FiltroEMA_Actualizar(FiltroEMA_t *f,
                               uint16_t nueva_muestra,
                               float alpha)
{
    if (!f->inicializado)
    {
        /* Primera muestra: inicializar con el valor real */
        f->valor_filtrado = (float)nueva_muestra;
        f->inicializado   = 1;
    }
    else
    {
        /* y[n] = alpha*x[n] + (1-alpha)*y[n-1] */
        f->valor_filtrado = alpha * (float)nueva_muestra
                          + (1.0f - alpha) * f->valor_filtrado;
    }
    return (uint16_t)f->valor_filtrado;
}

/* ============================================================
 * INSTANCIAS DE FILTROS — una por canal
 * ============================================================ */
static FiltroPromedio_t filtro_prom_pot;
static FiltroPromedio_t filtro_prom_ldr;
static FiltroPromedio_t filtro_prom_temp;

static FiltroEMA_t filtro_ema_pot;
static FiltroEMA_t filtro_ema_ldr;
static FiltroEMA_t filtro_ema_temp;

/* Inicializar todos los filtros — llamar en main() una vez */
void Ejercicio7_Filtros_Init(void)
{
    FiltroPromedio_Init(&filtro_prom_pot);
    FiltroPromedio_Init(&filtro_prom_ldr);
    FiltroPromedio_Init(&filtro_prom_temp);

    FiltroEMA_Init(&filtro_ema_pot);
    FiltroEMA_Init(&filtro_ema_ldr);
    FiltroEMA_Init(&filtro_ema_temp);
}

/* ============================================================
 * ESTRUCTURA COMPARATIVA — con y sin filtro
 * ============================================================ */
typedef struct {
    /* Sin filtro */
    uint16_t crudo_pot;
    uint16_t crudo_ldr;
    uint16_t crudo_temp;

    /* Promedio móvil */
    uint16_t prom_pot;
    uint16_t prom_ldr;
    uint16_t prom_temp;

    /* Filtro EMA */
    uint16_t ema_pot;
    uint16_t ema_ldr;
    uint16_t ema_temp;
} ComparacionFiltros_t;

/* Aplicar todos los filtros a las lecturas crudas */
void Ejercicio7_Aplicar_Filtros(ComparacionFiltros_t *c,
                                  uint16_t raw_pot,
                                  uint16_t raw_ldr,
                                  uint16_t raw_temp)
{
    /* Guardar valores crudos */
    c->crudo_pot  = raw_pot;
    c->crudo_ldr  = raw_ldr;
    c->crudo_temp = raw_temp;

    /* Promedio móvil */
    c->prom_pot   = FiltroPromedio_Actualizar(&filtro_prom_pot,  raw_pot);
    c->prom_ldr   = FiltroPromedio_Actualizar(&filtro_prom_ldr,  raw_ldr);
    c->prom_temp  = FiltroPromedio_Actualizar(&filtro_prom_temp, raw_temp);

    /* EMA */
    c->ema_pot    = FiltroEMA_Actualizar(&filtro_ema_pot,  raw_pot,  ALPHA_EMA);
    c->ema_ldr    = FiltroEMA_Actualizar(&filtro_ema_ldr,  raw_ldr,  ALPHA_EMA);
    c->ema_temp   = FiltroEMA_Actualizar(&filtro_ema_temp, raw_temp, ALPHA_EMA);
}

/* ============================================================
 * VISUALIZACIÓN COMPARATIVA EN LCD
 *
 * Alterna entre mostrar crudo vs filtrado cada 2 segundos
 * para que puedas comparar en el mismo LCD.
 * ============================================================ */
extern void LCD_SetCursor(uint8_t col, uint8_t row);
extern void LCD_Print(char *str);

void Ejercicio7_Mostrar_Comparacion(ComparacionFiltros_t *c)
{
    static uint32_t tick_cambio = 0;
    static uint8_t  modo        = 0;  // 0=crudo, 1=promedio, 2=EMA

    char linea0[17];
    char linea1[17];

    /* Cambiar modo cada 2 segundos */
    if ((HAL_GetTick() - tick_cambio) >= 2000)
    {
        tick_cambio = HAL_GetTick();
        modo = (modo + 1) % 3;
    }

    switch (modo)
    {
        case 0:  /* Sin filtro */
            snprintf(linea0, 17, "SIN FILTRO:");
            snprintf(linea1, 17, "P:%4u L:%4u T:%4u",
                     c->crudo_pot, c->crudo_ldr, c->crudo_temp);
            break;

        case 1:  /* Promedio móvil */
            snprintf(linea0, 17, "PROM MOV(N=%d):", VENTANA_PROMEDIO);
            snprintf(linea1, 17, "P:%4u L:%4u T:%4u",
                     c->prom_pot, c->prom_ldr, c->prom_temp);
            break;

        case 2:  /* EMA */
            snprintf(linea0, 17, "EMA(a=%.2f):", ALPHA_EMA);
            snprintf(linea1, 17, "P:%4u L:%4u T:%4u",
                     c->ema_pot, c->ema_ldr, c->ema_temp);
            break;
    }

    LCD_SetCursor(0, 0);
    LCD_Print(linea0);
    LCD_SetCursor(0, 1);
    LCD_Print(linea1);
}

/* ============================================================
 * EJEMPLO DE USO EN main.c
 * ============================================================
 *
 * // En main(), antes del while(1):
 * Ejercicio7_Filtros_Init();
 * HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_dma_buf, 3);
 *
 * // En el while(1):
 * ComparacionFiltros_t comparacion;
 * Ejercicio7_Aplicar_Filtros(&comparacion,
 *                              adc_dma_buf[0],
 *                              adc_dma_buf[1],
 *                              adc_dma_buf[2]);
 * Ejercicio7_Mostrar_Comparacion(&comparacion);
 * HAL_Delay(100);
 *
 * ============================================================
 * ANÁLISIS ESPERADO:
 *   - Sin filtro: valores fluctúan ±5-20 counts
 *   - Promedio móvil: muy estable, pero reacciona más lento
 *   - EMA con alpha bajo (0.05): muy suave, respuesta lenta
 *   - EMA con alpha alto (0.3):  más rápido, menos suavizado
 * ============================================================ */
