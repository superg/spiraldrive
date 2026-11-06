#ifndef PLEXTOR_LED_BOOT_DIAGNOSTICS_H
#define PLEXTOR_LED_BOOT_DIAGNOSTICS_H

#include "../platform/mcs251.h"

/* Diag13 stops immediately on received protocol=1/opcode=0x12, before
 * admission or transfer work. Other commands remain silent. */
#if defined(PLEXTOR_LED_BOOT_DIAGNOSTICS)
void LedBootDiagnosticArm(void);
void LedBootDiagnosticBeginCommand(void);
void LedBootDiagnosticMark(u8 stage);
void LedBootDiagnosticDataInExit(void);
void LedBootDiagnosticCommandExit(void);
void LedBootDiagnosticReport(void);
extern volatile u8 LedBootDiagnosticInquiryStage;
#define LED_BOOT_DIAGNOSTICS_ARM() LedBootDiagnosticArm()
#define LED_COMMAND_BEGIN() LedBootDiagnosticBeginCommand()
#define LED_DATA_IN_BEGIN() LedBootDiagnosticMark(2u)
#define LED_INQUIRY_MARK(stage) LedBootDiagnosticMark(stage)
#define LED_DATA_IN_EXIT() LedBootDiagnosticDataInExit()
#define LED_COMMAND_EXIT() LedBootDiagnosticCommandExit()
#define LED_INQUIRY_RESET() LedBootDiagnosticReport()
#else
#define LED_BOOT_DIAGNOSTICS_ARM() do { } while(0)
#define LED_COMMAND_BEGIN() do { } while(0)
#define LED_DATA_IN_BEGIN() do { } while(0)
#define LED_INQUIRY_MARK(stage) do { } while(0)
#define LED_DATA_IN_EXIT() do { } while(0)
#define LED_COMMAND_EXIT() do { } while(0)
#define LED_INQUIRY_RESET() do { } while(0)
#endif

#endif
