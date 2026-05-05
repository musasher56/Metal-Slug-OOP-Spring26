#pragma once

// WHY: Manual string helpers to avoid <cstring> dependency (strict constraint)
inline int strLen(const char* str) {
    int len = 0;
    while (str[len] != '\0') len++;
    return len;
}

inline void strCopy(char* dest, const char* src, int maxLen) {
    int i = 0;
    while (src[i] != '\0' && i < maxLen - 1) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

inline int strCompare(const char* s1, const char* s2) {
    int i = 0;
    while (s1[i] != '\0' && s2[i] != '\0') {
        if (s1[i] != s2[i]) return s1[i] - s2[i];
        i++;
    }
    return s1[i] - s2[i];
}
