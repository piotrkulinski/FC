#pragma once
#include "FiscalStatus.h"
#include "FiscalResult.h"
#include "EventType.h"

/**
 * \brief EVENT - Obsługa wydruków z terminali bez sprzężonego urządzenia drukującego, metoda callback przykazująca wydruk
 * \ingroup Enumerations
 * \param[in] char* context - Treść do wydrukowania
 * \param[out] type - typ komunikatu \ref TypeOnPrintData
*/
typedef int(CALLBACK* EventHandler)(const char* context, EventType type);
typedef int(CALLBACK* FoxEventHandler)(const char* context, EventType type);

/**
 * @brief Event do zmiany statusu
*/
typedef FiscalStatus(__stdcall* CallbackChangeStatus)(FiscalStatus state);
/**
 * @brief Event do zmiany rezultatu
*/
typedef FiscalResult(__stdcall* CallbackChangeResult)(FiscalResult result);

typedef bool(__stdcall* CallbackCheckStatus)(FiscalStatus result);
typedef bool(__stdcall* CallbackCheckResult)(FiscalResult result);
int CALLBACK TriggerCallbackSI(const char* message, EventType et);