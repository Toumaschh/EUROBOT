/*
 * VL53L5CX_LIB.h
 *
 *  Création : 9.01.2026
 *  Auteur : TFI
 *  Description : Librairie servant à utiliser le capteur VL53L5CX
 */


#ifndef VL53L5CX_LIB_H
#define VL53L5CX_LIB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "stm32h7xx_hal.h"
#include "custom_ranging_sensor.h"
#include <stdint.h>

// Définition
#define VL53L5CX_I2C_ADDR1_7BIT   (0x29U)
#define VL53L5CX_I2C_ADDR1_8BIT   (VL53L5CX_I2C_ADDR1_7BIT << 1)
#define VL53L5CX_I2C_ADDR2_7BIT   (0x2AU)
#define VL53L5CX_I2C_ADDR2_8BIT   (VL53L5CX_I2C_ADDR2_7BIT << 1)

// Variables globales externes
extern RANGING_SENSOR_Result_t resultat;

// Structure
// Orientation d'affichage de la matrice 8x8
typedef enum {
  VL53_ORIENT_STANDARD = 0,     // 0 = en haut à gauche
  VL53_ORIENT_FLIP_V,           // inverser verticalement
  VL53_ORIENT_FLIP_H,           // inverser horizontalement
  VL53_ORIENT_ROT_90_CW,        // rotation 90° horaire
  VL53_ORIENT_ROT_90_CCW        // rotation 90° anti-horaire
} VL53L5CX_Orientation;

// Style d'affichage (PuTTY)
typedef enum {
  VL53_PRINT_NUMERIC = 0,       // distance en mm, "--" si invalide
  VL53_PRINT_BG_COLOR           // fond coloré ANSI (plus lisible sur PuTTY)
} VL53L5CX_PrintStyle;


/**
 * @brief  Initialise l’I2C via BSP, séquence LPn/I2C_R, bus recovery (option),
 *         initialise + configure le VL53L5CX, puis démarre en continu (blocking).
 * @note   Avant d'appeler, assure-toi que HAL_Init(), SystemClock_Config(),
 *         MX_GPIO_Init() et BSP_COM_Init(COM1, ...) sont déjà faits.
 * @return BSP_ERROR_NONE si OK, sinon code d’erreur BSP/HAL.
 */
extern int32_t VL53L5CX_Init_All(void);

/**
 * @brief  Récupère la prochaine frame du capteur (bloquant en mode continu).
 * @param  res  Pointeur vers structure résultat
 * @return BSP_ERROR_NONE si OK.
 */
extern int32_t VL53L5CX_GetResult(RANGING_SENSOR_Result_t* res);

/**
 * @brief  Affiche la matrice 8x8 selon le style interne (numérique ou fond coloré).
 * @param  res  Résultat obtenu par VL53L5CX_GetResult
 */
extern int32_t VL53L5CX_PrintMatrix(void);

// (Optionnel) setters runtime si tu veux changer sans recompiler
extern void VL53L5CX_SetOrientation(VL53L5CX_Orientation o);
extern void VL53L5CX_SetPrintStyle(VL53L5CX_PrintStyle s);

#ifdef __cplusplus
}
#endif

#endif /* VL53L5CX_LIB_H */
