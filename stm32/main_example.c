/* USER CODE BEGIN Header */
/**
 **************************
 * @file           : main.c
 * @brief          : Main program body (EXEMPLO DE INTEGRACAO)
 **************************
 * @attention
 *
 * Este arquivo e um exemplo de integracao do algoritmo SAT/Integral Image
 * com STM32 HAL usando huart2. Copie apenas os trechos dentro de USER CODE
 * para o seu projeto gerado pelo CubeMX.
 *
 **************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdint.h>
#include <stddef.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define II_H 45
#define II_W 45
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
/*
 * Requisito de estrutura ~8KB no codigo da aplicacao (nao e lista de entrada):
 * 45*45*4 = 8100 bytes.
 */
static uint32_t sat[II_H][II_W];

/* Exemplo de imagem (entrada). Pode ser substituida por dados reais do sensor. */
static uint8_t img[II_H][II_W];

/* Buffers do seu exemplo (10 bytes). */
static uint8_t tx_buff[] = {65, 66, 67, 68, 69, 70, 71, 72, 73, 74};
static uint8_t rx_buff[10];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */
/* Prototipos do algoritmo (implementado no final deste arquivo). */
void ii_build_u8_fixed(const uint8_t img[II_H][II_W], uint32_t sat_out[II_H][II_W]);
uint32_t ii_sum_fixed(const uint32_t sat_in[II_H][II_W], int r1, int c1, int r2, int c2);

static void uart2_send_bytes(const uint8_t *data, uint16_t len);
static void uart2_send_str(const char *s);
static void uart2_send_u32_dec(uint32_t v);
static void fill_img_demo(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void uart2_send_bytes(const uint8_t *data, uint16_t len)
{
    if (!data || len == 0)
        return;

    /* Usa huart2 exatamente como configurado pelo CubeMX (USART2, 38400, 8N1). */
    (void)HAL_UART_Transmit(&huart2, (uint8_t *)data, len, 1000);
}

static void uart2_send_str(const char *s)
{
    if (!s)
        return;

    while (*s)
    {
        uint8_t ch = (uint8_t)*s++;
        uart2_send_bytes(&ch, 1);
    }
}

static void uart2_send_u32_dec(uint32_t v)
{
    /* Converte uint32_t para ASCII sem printf/sprintf (sem heap). */
    char buf[10];
    int i = 0;

    if (v == 0)
    {
        uart2_send_bytes((const uint8_t *)"0", 1);
        return;
    }

    while (v > 0 && i < (int)sizeof(buf))
    {
        buf[i++] = (char)('0' + (v % 10u));
        v /= 10u;
    }

    while (i-- > 0)
    {
        uint8_t ch = (uint8_t)buf[i];
        uart2_send_bytes(&ch, 1);
    }
}

static void fill_img_demo(void)
{
    /* Preenche a imagem de forma deterministica (2 lacos aninhados). */
    for (uint16_t r = 0; r < II_H; r++)
    {
        for (uint16_t c = 0; c < II_W; c++)
        {
            img[r][c] = (uint8_t)((r + c) & 0xFFu);
        }
    }
}
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
    /* USER CODE BEGIN 1 */
    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */
    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */
    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_USART1_UART_Init();
    MX_USART2_UART_Init();

    /* USER CODE BEGIN 2 */
    uart2_send_str("BOOT\r\n");

    /* 1) Gera uma imagem demo (substitua pelos seus dados reais). */
    fill_img_demo();

    /* 2) Constroi a Integral Image (SAT) no buffer de 8KB (sat[][]). */
    ii_build_u8_fixed(img, sat);
    uart2_send_str("SAT_READY\r\n");

    /* 3) Faz algumas consultas O(1) e envia via UART2. */
    {
        uint32_t s0 = ii_sum_fixed(sat, 0, 0, II_H - 1, II_W - 1);
        uint32_t s1 = ii_sum_fixed(sat, 10, 10, 20, 20);

        uart2_send_str("SUM_ALL=");
        uart2_send_u32_dec(s0);
        uart2_send_str("\r\n");

        uart2_send_str("SUM_10_10_20_20=");
        uart2_send_u32_dec(s1);
        uart2_send_str("\r\n");
    }
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
        /* Mantem uma transmissao periodica usando o seu huart2. */
        (void)HAL_UART_Transmit(&huart2, tx_buff, 10, 1000);
        HAL_Delay(1000);

        /* Exemplo (opcional): receber comandos/queries em rx_buff se quiser. */
        (void)HAL_UART_Receive(&huart2, rx_buff, (uint16_t)sizeof(rx_buff), 10);
        /* USER CODE END 3 */
    }
    /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
    {
        Error_Handler();
    }
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
    PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief USART1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART1_UART_Init(void)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief USART2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART2_UART_Init(void)
{
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 38400;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&huart2) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    (void)file;
    (void)line;
}
#endif /* USE_FULL_ASSERT */

/* USER CODE BEGIN 4 */
/* -------------------------------------------------------------------------- */
/* Algoritmo: Integral Image / Summed Area Table (SAT)                         */
/* - Sem recursao                                                              */
/* - Sem alocacao dinamica                                                     */
/* - Construcao: 2 lacos aninhados O(II_H * II_W)                              */
/* - Consulta: O(1)                                                            */
/* -------------------------------------------------------------------------- */

typedef uint64_t ii_wide_t;

void ii_build_u8_fixed(const uint8_t img_in[II_H][II_W], uint32_t sat_out[II_H][II_W])
{
    if (!img_in || !sat_out)
        return;

    for (uint16_t r = 0; r < (uint16_t)II_H; r++)
    {
        ii_wide_t row_running_sum = 0;

        for (uint16_t c = 0; c < (uint16_t)II_W; c++)
        {
            row_running_sum += (ii_wide_t)img_in[r][c];
            sat_out[r][c] = (uint32_t)(row_running_sum + ((r > 0U) ? (ii_wide_t)sat_out[r - 1U][c] : (ii_wide_t)0));
        }
    }
}

uint32_t ii_sum_fixed(const uint32_t sat_in[II_H][II_W], int r1, int c1, int r2, int c2)
{
    if (!sat_in)
        return 0U;

    if (r1 < 0 || c1 < 0 || r2 < 0 || c2 < 0)
        return 0U;

    if (r1 >= (int)II_H || r2 >= (int)II_H || c1 >= (int)II_W || c2 >= (int)II_W)
        return 0U;

    if (r1 > r2 || c1 > c2)
        return 0U;

    /* soma = A - B - C + D */
    ii_wide_t A = (ii_wide_t)sat_in[r2][c2];
    ii_wide_t B = (r1 > 0) ? (ii_wide_t)sat_in[r1 - 1][c2] : (ii_wide_t)0;
    ii_wide_t C = (c1 > 0) ? (ii_wide_t)sat_in[r2][c1 - 1] : (ii_wide_t)0;
    ii_wide_t D = (r1 > 0 && c1 > 0) ? (ii_wide_t)sat_in[r1 - 1][c1 - 1] : (ii_wide_t)0;

    return (uint32_t)(A - B - C + D);
}
/* USER CODE END 4 */
