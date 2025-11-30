/* File: src/buddy/config.h */
#ifndef CONFIG_H
#define CONFIG_H

// --- PHẦN 1: Tắt cảnh báo "Unsafe" của Visual Studio (Sửa lỗi C4996) ---
// Phải đặt dòng này lên đầu tiên
#define _CRT_SECURE_NO_WARNINGS 1

// --- PHẦN 2: Các thư viện chuẩn ---
#define STDC_HEADERS 1
#define HAVE_STDLIB_H 1
#define HAVE_STRING_H 1
#define HAVE_LIMITS_H 1


// kernel.c cần biết phiên bản của thư viện là gì
#define PACKAGE_VERSION "2.4"
#define MAJOR_VERSION 2
#define MINOR_VERSION 4

#endif // CONFIG_H