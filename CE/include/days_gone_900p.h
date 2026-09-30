#ifndef DAYS_GONE_900P_H
#define DAYS_GONE_900P_H

#include <stdint.h>
#include <stddef.h>

/* Orbis kernel types */
typedef int64_t ssize_t;
typedef uint64_t size_t;

/* Function pointer types for UE4 functions we'll hook */
typedef void (*SetResolutionScaleNormalized_t)(void* GameUserSettings, float value);
typedef void (*SetResolutionScaleValue_t)(void* GameUserSettings, float value);
typedef void (*SetScreenResolution_t)(void* GameUserSettings, int width, int height);
typedef void (*SetShadowQuality_t)(void* GameUserSettings, int quality);
typedef void (*SetViewDistanceQuality_t)(void* GameUserSettings, int quality);
typedef void (*SetPostProcessingQuality_t)(void* GameUserSettings, int quality);
typedef void (*ApplyResolutionSettings_t)(void* GameUserSettings);
typedef void* (*GetGameUserSettings_t)(void);

/* Global function pointers (resolved at runtime) */
extern SetResolutionScaleNormalized_t g_SetResolutionScaleNormalized;
extern SetResolutionScaleValue_t g_SetResolutionScaleValue;
extern SetScreenResolution_t g_SetScreenResolution;
extern SetShadowQuality_t g_SetShadowQuality;
extern SetViewDistanceQuality_t g_SetViewDistanceQuality;
extern SetPostProcessingQuality_t g_SetPostProcessingQuality;
extern ApplyResolutionSettings_t g_ApplyResolutionSettings;
extern GetGameUserSettings_t g_GetGameUserSettings;

/* Pattern scan results */
extern void* g_GameUserSettings_vtable;

/* Initialize all hooks */
int days_gone_900p_init(void);

/* Apply 900p + quality reductions */
void days_gone_900p_apply_settings(void);

/* Debug logging */
void klog(const char* fmt, ...);

#endif