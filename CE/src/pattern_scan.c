#include "days_gone_900p.h"
#include <string.h>

/* Pattern scan utilities for finding functions in memory */

/* Known string patterns from the eboot.bin analysis */
/* These strings are in the .rodata section and are referenced by the functions */
static const struct {
    const char* name;
    const char* string_pattern;
    size_t string_len;
} g_target_strings[] = {
    {"SetResolutionScaleNormalized", "SetResolutionScaleNormalized", 28},
    {"SetResolutionScaleValue", "SetResolutionScaleValue", 23},
    {"SetScreenResolution", "SetScreenResolution", 19},
    {"SetShadowQuality", "SetShadowQuality", 16},
    {"SetViewDistanceQuality", "SetViewDistanceQuality", 22},
    {"SetPostProcessingQuality", "SetPostProcessingQuality", 24},
    {"ApplyResolutionSettings", "ApplyResolutionSettings", 21},
    {"GetGameUserSettings", "GetGameUserSettings", 19},
    {"GameUserSettings", "GameUserSettings", 16},
    {NULL, NULL, 0}
};

/* x86-64 instruction patterns for common UE4 setter functions */
/* These are typical prologues we can search for near string references */

typedef struct {
    uint64_t address;
    const char* name;
} pattern_match_t;

/* Scan memory region for a byte pattern */
static void* pattern_scan(void* start, size_t len, const uint8_t* pattern, size_t pattern_len, const uint8_t* mask) {
    uint8_t* ptr = (uint8_t*)start;
    uint8_t* end = ptr + len - pattern_len;
    
    while (ptr <= end) {
        int match = 1;
        for (size_t i = 0; i < pattern_len; i++) {
            if (mask && !(mask[i] & 0x1)) continue;  /* Wildcard */
            if (ptr[i] != pattern[i]) {
                match = 0;
                break;
            }
        }
        if (match) {
            return ptr;
        }
        ptr++;
    }
    return NULL;
}

/* Find string in memory and return its address */
static void* find_string(void* start, size_t len, const char* str, size_t str_len) {
    return pattern_scan(start, len, (const uint8_t*)str, str_len, NULL);
}

/* Find RIP-relative reference to a string */
static uint64_t find_rip_relative_ref(void* code_start, size_t code_len, uint64_t string_addr) {
    uint8_t* ptr = (uint8_t*)code_start;
    uint8_t* end = ptr + code_len - 7;
    
    while (ptr <= end) {
        /* LEA reg, [rip + disp32] : 48 8d xx 05 disp32 */
        /* or 4c 8d xx 05 disp32 */
        if ((ptr[0] == 0x48 || ptr[0] == 0x4c) && ptr[1] == 0x8d && ptr[2] == 0x05) {
            int32_t disp32 = *(int32_t*)(ptr + 3);
            uint64_t instr_addr = (uint64_t)ptr;
            uint64_t target = instr_addr + 7 + disp32;
            if (target == string_addr) {
                return instr_addr;
            }
        }
        /* MOV reg, [rip + disp32] : 48 8b xx 05 disp32 */
        if (ptr[0] == 0x48 && ptr[1] == 0x8b && ptr[2] == 0x05) {
            int32_t disp32 = *(int32_t*)(ptr + 3);
            uint64_t instr_addr = (uint64_t)ptr;
            uint64_t target = instr_addr + 7 + disp32;
            if (target == string_addr) {
                return instr_addr;
            }
        }
        ptr++;
    }
    return 0;
}

/* Find function start by scanning backwards for prologue */
static uint64_t find_function_start(void* code_start, size_t code_len, uint64_t instr_addr) {
    uint8_t* start = (uint8_t*)code_start;
    uint8_t* instr = (uint8_t*)instr_addr;
    uint8_t* scan = instr;
    
    /* Scan backwards up to 512 bytes */
    for (int i = 0; i < 512 && scan > start; i++, scan--) {
        /* push rbp; mov rbp, rsp */
        if (scan[0] == 0x55 && scan[1] == 0x48 && scan[2] == 0x89 && scan[3] == 0xe5) {
            return (uint64_t)scan;
        }
        /* push rbx; push rbp; mov rbp, rsp */
        if (scan[0] == 0x53 && scan[1] == 0x55 && scan[2] == 0x48 && scan[3] == 0x89 && scan[4] == 0xe5) {
            return (uint64_t)scan;
        }
    }
    return 0;
}

/* Resolve all UE4 graphics settings functions */
int resolve_ue4_functions(void) {
    klog("[days_gone_900p] Starting pattern scan...\n");
    
    /* Memory regions (typical PS4 layout for this game) */
    /* Code segment: 0x0 - 0x5724830 (from ELF analysis) */
    void* code_base = (void*)0x0;
    size_t code_size = 0x5724830;
    
    /* String table region: ~0x4a80000 - 0x4a90000 */
    void* string_base = (void*)0x4a80000;
    size_t string_size = 0x100000;
    
    int resolved = 0;
    
    for (int i = 0; g_target_strings[i].name; i++) {
        const char* str = g_target_strings[i].string_pattern;
        size_t str_len = g_target_strings[i].string_len;
        
        /* Find the string in memory */
        void* str_addr = find_string(string_base, string_size, str, str_len);
        if (!str_addr) {
            klog("[days_gone_900p] String not found: %s\n", str);
            continue;
        }
        
        uint64_t str_vaddr = (uint64_t)str_addr;
        klog("[days_gone_900p] Found string '%s' at 0x%lx\n", str, str_vaddr);
        
        /* Find RIP-relative reference to this string in code */
        uint64_t ref_addr = find_rip_relative_ref(code_base, code_size, str_vaddr);
        if (!ref_addr) {
            klog("[days_gone_900p] No code reference to string: %s\n", str);
            continue;
        }
        
        klog("[days_gone_900p] Found reference at 0x%lx\n", ref_addr);
        
        /* Find function start */
        uint64_t func_addr = find_function_start(code_base, code_size, ref_addr);
        if (!func_addr) {
            klog("[days_gone_900p] No function prologue found for: %s\n", str);
            continue;
        }
        
        klog("[days_gone_900p] Function '%s' at 0x%lx\n", str, func_addr);
        
        /* Assign to appropriate function pointer */
        if (strcmp(str, "SetResolutionScaleNormalized") == 0) {
            g_SetResolutionScaleNormalized = (SetResolutionScaleNormalized_t)func_addr;
            resolved++;
        } else if (strcmp(str, "SetResolutionScaleValue") == 0) {
            g_SetResolutionScaleValue = (SetResolutionScaleValue_t)func_addr;
            resolved++;
        } else if (strcmp(str, "SetScreenResolution") == 0) {
            g_SetScreenResolution = (SetScreenResolution_t)func_addr;
            resolved++;
        } else if (strcmp(str, "SetShadowQuality") == 0) {
            g_SetShadowQuality = (SetShadowQuality_t)func_addr;
            resolved++;
        } else if (strcmp(str, "SetViewDistanceQuality") == 0) {
            g_SetViewDistanceQuality = (SetViewDistanceQuality_t)func_addr;
            resolved++;
        } else if (strcmp(str, "SetPostProcessingQuality") == 0) {
            g_SetPostProcessingQuality = (SetPostProcessingQuality_t)func_addr;
            resolved++;
        } else if (strcmp(str, "ApplyResolutionSettings") == 0) {
            g_ApplyResolutionSettings = (ApplyResolutionSettings_t)func_addr;
            resolved++;
        } else if (strcmp(str, "GetGameUserSettings") == 0) {
            g_GetGameUserSettings = (GetGameUserSettings_t)func_addr;
            resolved++;
        } else if (strcmp(str, "GameUserSettings") == 0) {
            /* This string is in the vtable area - find vtable */
            /* Look for vtable pointer near this string */
            g_GameUserSettings_vtable = str_addr;  /* placeholder */
        }
    }
    
    klog("[days_gone_900p] Resolved %d functions\n", resolved);
    return resolved > 0 ? 0 : -1;
}