#include "days_gone_900p.h"
#include <stdint.h>

/* Target settings for 900p */
/* 1080p = 1920x1080 */
/* 900p = 1600x900 (or 1920x1080 with 75% resolution scale) */
/* ScreenPercentage: 100 = native, 75 = 900p from 1080p */

#define TARGET_SCREEN_PERCENTAGE 75.0f   /* 75% = 900p from 1080p */
#define TARGET_SHADOW_QUALITY 1          /* 0=Low, 1=Medium, 2=High, 3=Epic */
#define TARGET_VIEW_DISTANCE_QUALITY 1   /* 0=Low, 1=Medium, 2=High, 3=Epic */
#define TARGET_POST_PROCESS_QUALITY 1    /* 0=Low, 1=Medium, 2=High, 3=Epic */

static int g_settings_applied = 0;

void days_gone_900p_apply_settings(void) {
    if (g_settings_applied) {
        klog("[days_gone_900p] Settings already applied\n");
        return;
    }
    
    klog("[days_gone_900p] Applying 900p + quality reduction settings...\n");
    
    /* Get GameUserSettings instance */
    void* gus = NULL;
    if (g_GetGameUserSettings) {
        gus = g_GetGameUserSettings();
        klog("[days_gone_900p] GameUserSettings instance: %p\n", gus);
    } else {
        klog("[days_gone_900p] GetGameUserSettings not resolved!\n");
        return;
    }
    
    if (!gus) {
        klog("[days_gone_900p] GameUserSettings is NULL!\n");
        return;
    }
    
    /* Apply resolution scale (900p via ScreenPercentage) */
    if (g_SetResolutionScaleNormalized) {
        /* Normalized: 1.0 = 100%, 0.75 = 75% */
        float normalized = TARGET_SCREEN_PERCENTAGE / 100.0f;
        g_SetResolutionScaleNormalized(gus, normalized);
        klog("[days_gone_900p] SetResolutionScaleNormalized(%.2f)\n", normalized);
    } else if (g_SetResolutionScaleValue) {
        /* Absolute value in some units */
        g_SetResolutionScaleValue(gus, TARGET_SCREEN_PERCENTAGE);
        klog("[days_gone_900p] SetResolutionScaleValue(%d)\n", TARGET_SCREEN_PERCENTAGE);
    } else if (g_SetScreenResolution) {
        /* Direct resolution - 1600x900 for true 900p */
        g_SetScreenResolution(gus, 1600, 900);
        klog("[days_gone_900p] SetScreenResolution(1600, 900)\n");
    } else {
        klog("[days_gone_900p] No resolution setter available!\n");
    }
    
    /* Reduce shadow quality */
    if (g_SetShadowQuality) {
        g_SetShadowQuality(gus, TARGET_SHADOW_QUALITY);
        klog("[days_gone_900p] SetShadowQuality(%d)\n", TARGET_SHADOW_QUALITY);
    } else {
        klog("[days_gone_900p] SetShadowQuality not available\n");
    }
    
    /* Reduce view distance quality (affects fog/LOD distance) */
    if (g_SetViewDistanceQuality) {
        g_SetViewDistanceQuality(gus, TARGET_VIEW_DISTANCE_QUALITY);
        klog("[days_gone_900p] SetViewDistanceQuality(%d)\n", TARGET_VIEW_DISTANCE_QUALITY);
    } else {
        klog("[days_gone_900p] SetViewDistanceQuality not available\n");
    }
    
    /* Reduce post-processing quality (affects fog, bloom, DOF, etc.) */
    if (g_SetPostProcessingQuality) {
        g_SetPostProcessingQuality(gus, TARGET_POST_PROCESS_QUALITY);
        klog("[days_gone_900p] SetPostProcessingQuality(%d)\n", TARGET_POST_PROCESS_QUALITY);
    } else {
        klog("[days_gone_900p] SetPostProcessingQuality not available\n");
    }
    
    /* Apply the resolution settings */
    if (g_ApplyResolutionSettings) {
        g_ApplyResolutionSettings(gus);
        klog("[days_gone_900p] ApplyResolutionSettings()\n");
    }
    
    g_settings_applied = 1;
    klog("[days_gone_900p] Settings applied successfully\n");
}

/* Re-apply settings (in case game resets them) */
void days_gone_900p_reapply_settings(void) {
    g_settings_applied = 0;
    days_gone_900p_apply_settings();
}

/* Initialize the patch */
int days_gone_900p_init(void) {
    klog("[days_gone_900p] Initializing...\n");
    
    /* Resolve UE4 functions via pattern scanning */
    int ret = resolve_ue4_functions();
    if (ret < 0) {
        klog("[days_gone_900p] Failed to resolve UE4 functions\n");
        return -1;
    }
    
    klog("[days_gone_900p] Function resolution complete\n");
    
    /* Try to hook game tick to re-apply settings periodically */
    /* hook_gamesettings_tick(); */
    
    return 0;
}