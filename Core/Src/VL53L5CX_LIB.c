/*
 * VL53L5CX_LIB.c
 *
 *  Created on: 9 janv. 2026
 *      Author: TFI
 */



#include "vl53l5cx_lib.h"
#include "stm32h7xx_nucleo_bus.h"    /* BSP_I2C1_Init, hi2c1 */
#include "main.h"                    /* pour LPn_Pin, I2C_R_Pin, PWREN_Pin, ports */
#include <stdio.h>
#include <string.h>

/* =========================
 * Configuration interne (modifiable si besoin)
 * ========================= */

/* Profil capteur */
#define VL53_DEFAULT_PROFILE       RS_PROFILE_8x8_CONTINUOUS
#define VL53_DEFAULT_TB_MS         (30)   /* Timing budget (ms) */
#define VL53_DEFAULT_FREQ_HZ       (10)   /* Fréquence (Hz) */
#define VL53_DEFAULT_AMBIENT       (1)    /* 1 = enable ambient */
#define VL53_DEFAULT_SIGNAL        (1)    /* 1 = enable signal */

/* I2C helpers */
#define VL53_USE_I2C_RECOVERY      (1)    /* 1 = faire une recovery PB8/PB9 avant init BSP */
#define VL53_DO_I2C_SCAN_AND_PROBE (1)    /* 1 = scan + probe + ID brut (debug) */

/* Affichage */
static VL53L5CX_Orientation g_orientation = VL53_ORIENT_FLIP_H;
static VL53L5CX_PrintStyle  g_print_style = VL53_PRINT_BG_COLOR; /* Putty: fond coloré par défaut */

/* Le handle I2C1 est DÉFINI dans le BSP */
extern I2C_HandleTypeDef hi2c1;

/* =========================
 * ANSI pour PuTTY (fond coloré)
 * ========================= */
#define ANSI_RESET   "\x1b[0m"
#define FG_BLACK     "\x1b[30m"
#define FG_WHITE     "\x1b[37m"
#define BG_GREEN     "\x1b[42m"
#define BG_YELLOW    "\x1b[43m"
#define BG_RED       "\x1b[41m"
#define BG_BLUE      "\x1b[44m"

// Variables globales externes
RANGING_SENSOR_Result_t resultat;


/* =========================
 * Prototypes internes
 * ========================= */
static void prv_configure_ctrl_pins(void);
static void prv_powerup_sequence(void);
static void prv_i2c_bus_recovery_pb8_pb9(void);
static int  prv_idx_from_rc(int r, int c);
static void prv_print_matrix_numeric(const RANGING_SENSOR_Result_t* res);
static void prv_print_matrix_bgcolor(const RANGING_SENSOR_Result_t* res);

/* =========================
 * API
 * ========================= */
int32_t VL53L5CX_Init_All(void)
{
	printf("\r\n=== Boot ===\r\n");

	/* 1) Broches de contrôle capteur (LPn/I2C_R/PWREN) */
	prv_configure_ctrl_pins();

	/* 2) Séquence power-up capteur */
	prv_powerup_sequence();

	/* 3) Recovery I2C (option) */
	if (VL53_USE_I2C_RECOVERY)
	{
		prv_i2c_bus_recovery_pb8_pb9();
	}

	/* 4) Init I2C via BSP */
	if (BSP_I2C1_Init() != BSP_ERROR_NONE)
	{
		printf("BSP_I2C1_Init failed!\r\n");
		return BSP_ERROR_PERIPH_FAILURE;
	}
	HAL_Delay(50);

	/* 5) Debug : Scan / Probe / ID brut (optionnel) */
	if (VL53_DO_I2C_SCAN_AND_PROBE)
	{
		printf("I2C scan...\r\n");
		int found = 0;

		for (uint16_t addr = 1; addr < 127; addr++)
		{
			if (HAL_I2C_IsDeviceReady(&hi2c1, (addr << 1), 2, 50) == HAL_OK)
			{
				printf("  -> ACK @ 0x%02X\r\n", addr);
				found = 1;
			}
		}

		if (!found)
		{
			printf("  Aucun périphérique détecté\r\n");
		}

		printf("Probe 0x29...\r\n");
		uint8_t dummy = 0;
		HAL_StatusTypeDef pr = HAL_I2C_Master_Transmit(&hi2c1, VL53L5CX_I2C_ADDR1_8BIT, &dummy, 1, 100);
		printf("  Probe result: %d, I2C Err=0x%lx\r\n", pr, HAL_I2C_GetError(&hi2c1));

		uint8_t id_data[2] = {0};

		if (HAL_I2C_Mem_Read(&hi2c1, VL53L5CX_I2C_ADDR1_8BIT, 0x010F, I2C_MEMADD_SIZE_16BIT, id_data, 2, 100) == HAL_OK)
		{
			printf("  Raw ID: 0x%02X%02X\r\n", id_data[0], id_data[1]);
		}
		else
		{
			printf("  Raw ID read failed, Err=0x%lx\r\n", HAL_I2C_GetError(&hi2c1));
		}

	}

	/* 6) Init driver capteur */
	int32_t st = CUSTOM_RANGING_SENSOR_Init(CUSTOM_VL53L5CX);

	if (st != BSP_ERROR_NONE)
	{
		printf("VL53 Init failed, st=%ld\r\n", st);
		return st;
	}

	/* 7) Config profil (interne) */
	RANGING_SENSOR_ProfileConfig_t profile;
	profile.RangingProfile = VL53_DEFAULT_PROFILE;
	profile.TimingBudget   = VL53_DEFAULT_TB_MS;
	profile.Frequency      = VL53_DEFAULT_FREQ_HZ;
	profile.EnableAmbient  = VL53_DEFAULT_AMBIENT;
	profile.EnableSignal   = VL53_DEFAULT_SIGNAL;

	st = CUSTOM_RANGING_SENSOR_ConfigProfile(CUSTOM_VL53L5CX, &profile);

	if (st != BSP_ERROR_NONE)
	{
		printf("VL53L5CX ConfigProfile failed, st=%ld\r\n", st);
		return st;
	}

	/* 8) Start en continu (blocking) */
	st = CUSTOM_RANGING_SENSOR_Start(CUSTOM_VL53L5CX, RS_MODE_BLOCKING_CONTINUOUS);
	if (st != BSP_ERROR_NONE)
	{
		printf("VL53L5CX Start failed, st=%ld\r\n", st);
		return st;
	}

	printf("VL53L5CX ready: profile=8x8, %uHz, TB=%ums\r\n",
			(unsigned)VL53_DEFAULT_FREQ_HZ,
			(unsigned)VL53_DEFAULT_TB_MS);

	return BSP_ERROR_NONE;
}

int32_t VL53L5CX_GetResult(RANGING_SENSOR_Result_t* res)
{
	if (res == 0)
	{
		return BSP_ERROR_WRONG_PARAM;
	}

	return CUSTOM_RANGING_SENSOR_GetDistance(CUSTOM_VL53L5CX, res);
}


int32_t VL53L5CX_PrintMatrix(void)
{
	// Variables locales
	static uint32_t frame_counter = 0;

	// 1) Lire la prochaine frame dans la GLOBALE
	int32_t st = VL53L5CX_GetResult(&resultat);
	if (st != BSP_ERROR_NONE)
	{
		printf("GetResukt échoué\r\n");
		return st;
	}

	// 2) Throttle (~1 fps) pour éviter le spam UART
	if ((frame_counter++ % 10) != 0)
	{
		return BSP_ERROR_NONE;
	}

	printf("\r\n=== Frame %lu ===\r\n", frame_counter / 10);
	printf("Zones: %ld\r\n", resultat.NumberOfZones);

	// 3) Affichage
	switch (g_print_style)
	{
    	case VL53_PRINT_NUMERIC:
    		prv_print_matrix_numeric(&resultat);
    		break;
    	case VL53_PRINT_BG_COLOR:
    		prv_print_matrix_bgcolor(&resultat);
    		break;
    	default:
    		prv_print_matrix_numeric(&resultat);
    		break;
	}

	return BSP_ERROR_NONE;
}


void VL53L5CX_SetOrientation(VL53L5CX_Orientation o)
{
  g_orientation = o;
}

void VL53L5CX_SetPrintStyle(VL53L5CX_PrintStyle s)
{
  g_print_style = s;
}

/* =========================
 * Internes
 * ========================= */

static void prv_configure_ctrl_pins(void)
{
  GPIO_InitTypeDef gi = {0};

  /* PWREN (si macro existante dans main.h) */
#ifdef PWREN_Pin
  gi.Pin   = PWREN_Pin;
  gi.Mode  = GPIO_MODE_OUTPUT_PP;
  gi.Pull  = GPIO_PULLUP;
  gi.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(PWREN_GPIO_Port, &gi);
  HAL_GPIO_WritePin(PWREN_GPIO_Port, PWREN_Pin, GPIO_PIN_SET); /* Power on */
#endif

  /* I2C_R (si pont I2C présent) */
#ifdef I2C_R_Pin
  gi.Pin   = I2C_R_Pin;
  gi.Mode  = GPIO_MODE_OUTPUT_PP;
  gi.Pull  = GPIO_NOPULL;
  gi.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(I2C_R_GPIO_Port, &gi);
  HAL_GPIO_WritePin(I2C_R_GPIO_Port, I2C_R_Pin, GPIO_PIN_RESET); /* relâché par défaut */
#endif

  /* LPn (enable capteur) */
#ifdef LPn_Pin
  gi.Pin   = LPn_Pin;
  gi.Mode  = GPIO_MODE_OUTPUT_PP;
  gi.Pull  = GPIO_NOPULL;
  gi.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LPn_GPIO_Port, &gi);
  HAL_GPIO_WritePin(LPn_GPIO_Port, LPn_Pin, GPIO_PIN_RESET); /* capteur off par défaut */
#endif
}

static void prv_powerup_sequence(void)
{
#ifdef I2C_R_Pin
  HAL_GPIO_WritePin(I2C_R_GPIO_Port, I2C_R_Pin, GPIO_PIN_SET);   /* bridge en reset */
#endif
#ifdef LPn_Pin
  HAL_GPIO_WritePin(LPn_GPIO_Port, LPn_Pin, GPIO_PIN_RESET);     /* capteur off */
#endif
  HAL_Delay(10);

#ifdef I2C_R_Pin
  HAL_GPIO_WritePin(I2C_R_GPIO_Port, I2C_R_Pin, GPIO_PIN_RESET); /* relâche bridge */
#endif
  HAL_Delay(10);

#ifdef LPn_Pin
  HAL_GPIO_WritePin(LPn_GPIO_Port, LPn_Pin, GPIO_PIN_SET);       /* capteur on */
#endif
  HAL_Delay(10);
}

static void prv_i2c_bus_recovery_pb8_pb9(void)
{
  /* Recovery "bit-bang" sur PB8 (SCL) & PB9 (SDA) */
  GPIO_InitTypeDef gi = {0};

  __HAL_RCC_I2C1_FORCE_RESET();
  HAL_Delay(2);

  __HAL_RCC_I2C1_RELEASE_RESET();
  HAL_Delay(2);

  __HAL_RCC_I2C1_CLK_DISABLE();

  __HAL_RCC_GPIOB_CLK_ENABLE();

  gi.Pin   = GPIO_PIN_8 | GPIO_PIN_9;
  gi.Mode  = GPIO_MODE_OUTPUT_OD;
  gi.Pull  = GPIO_PULLUP;
  gi.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &gi);

  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET); /* SDA HIGH */

  for (int i = 0; i < 9; i++)
  {
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
	  HAL_Delay(1);

	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
	  HAL_Delay(1);
  }

  /* STOP: SCL low, SDA low -> SCL high -> SDA high */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
  HAL_Delay(1);

  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);
  HAL_Delay(1);

  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
  HAL_Delay(1);

  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
  HAL_Delay(1);
}

static int prv_idx_from_rc(int r, int c)
{
  switch (g_orientation) {
    default:
    case VL53_ORIENT_STANDARD:   return (r * 8 + c);
    case VL53_ORIENT_FLIP_V:     return ((7 - r) * 8 + c);
    case VL53_ORIENT_FLIP_H:     return (r * 8 + (7 - c));
    case VL53_ORIENT_ROT_90_CW:  return (c * 8 + (7 - r));
    case VL53_ORIENT_ROT_90_CCW: return ((7 - c) * 8 + r);
  }
}

static void prv_print_matrix_numeric(const RANGING_SENSOR_Result_t* res)
{
	if (res->NumberOfZones < 64)
	{
		printf("Matrice invalide: %ld zones\r\n", res->NumberOfZones);
		return;
	}

	printf("Matrice 8x8 (mm)\r\n");
	for (int r = 0; r < 8; r++)
 	{
		for (int c = 0; c < 8; c++)
		{
			int idx = prv_idx_from_rc(r, c);
			long d = res->ZoneResult[idx].Distance[0];
			long s = res->ZoneResult[idx].Status[0];

			if (s != 0)
			{
				printf("  -- ");
			}
			else
			{
				printf("%4ld ", d);
			}
		}

		printf("\r\n");

 	}

	printf("\r\n");

}

static const char* prv_bg_for_cell(long d, long status)
{
	if (status != 0)
	{
	 	return BG_BLUE;
	}

	if (d < 300)
	{
		return BG_RED;
	}

	if (d < 1000)
	{
		return BG_YELLOW;
	}

	return BG_GREEN;
}

static void prv_print_matrix_bgcolor(const RANGING_SENSOR_Result_t* res)
{
	if (res->NumberOfZones < 64)
	{
		printf("Matrice invalide: %ld zones\r\n", res->NumberOfZones);
		return;
	}

	printf("Matrice 8x8 (fond=distance)\r\n");

	for (int r = 0; r < 8; r++)
	{
		for (int c = 0; c < 8; c++)
		{
			int idx = prv_idx_from_rc(r, c);
			long d = res->ZoneResult[idx].Distance[0];
			long s = res->ZoneResult[idx].Status[0];
			const char* bg = prv_bg_for_cell(d, s);

			if (s != 0)
			{
				printf("%s%s  -- %s ", bg, FG_WHITE, ANSI_RESET);
			}
			else
			{
				printf("%s%s%4ld%s ", bg, FG_BLACK, d, ANSI_RESET);
			}
		}

		printf("\r\n");

	}

	printf("\r\n");

}
