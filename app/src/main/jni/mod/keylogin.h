#ifndef KEYLOGIN_H
#define KEYLOGIN_H

#include <curl/curl.h>
#include <jni.h>
#include <string>
#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <vector>
#include <json/json.hpp>

// ============================================
// إعدادات السيرفر
// ============================================
const std::string API_BASE_URL = "https://venomkey.com/connect"; 
const std::string DEFAULT_GAME = "pubg";

// ============================================
// تعريف المتغيرات العامة
// ============================================
bool bValid = false;
bool logged_in = false;
bool is_logging_in = false;
std::string g_Token = "";
std::string g_Auth = "";
std::string g_ExpTime = "N/A";
std::string ERROR_MESSAGE = "";
time_t g_ServerExpiry = 0; // Added to fix menu.h error

// ============================================
// دالة تشفير بسيطة
// ============================================
std::string xor_encrypt(std::string data) {
    char key = 'H'; // مفتاح سري
    for (size_t i = 0; i < data.size(); i++)
        data[i] = data[i] ^ key;
    return data;
}

// ============================================
// دالة httpPost
// ============================================
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
    response->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string httpPost(const std::string& url, const std::string& postData) {
    CURL* curl = curl_easy_init();
    std::string response;
    if (!curl) return "";
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "HassanLoader/1.0");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); 
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    return response;
}

// ============================================
// تحليل JSON يدوي بسيط
// ============================================
std::string extractJsonString(const std::string& json, const std::string& key) {
    std::string searchKey = "\"" + key + "\"";
    size_t keyPos = json.find(searchKey);
    if (keyPos == std::string::npos) return "";
    
    size_t valuePos = json.find(":", keyPos);
    if (valuePos == std::string::npos) return "";
    
    size_t quote1 = json.find("\"", valuePos);
    if (quote1 == std::string::npos) return "";
    
    size_t quote2 = json.find("\"", quote1 + 1);
    if (quote2 == std::string::npos) return "";
    
    return json.substr(quote1 + 1, quote2 - quote1 - 1);
}

// Simple Expiry Parser for timestamp
static time_t ParseExpiryTime(const std::string& exp) {
    if (exp.empty() || exp == "N/A") return 0;
    try {
        size_t pos = 0;
        long long timestamp = std::stoll(exp, &pos);
        if (pos == exp.size() && timestamp > 1000000000LL) {
            return static_cast<time_t>(timestamp);
        }
    } catch (...) {}
    return 0;
}

// ============================================
// دالة التحقق المحدثة
// ============================================
bool verifyWithYourServer(const std::string& license_key, const std::string& hwid) {
    std::string postData = "game=" + DEFAULT_GAME + "&user_key=" + license_key + "&serial=" + hwid;
    
    std::string response = httpPost(API_BASE_URL, postData);
    
    if (response.find("\"status\":true") != std::string::npos || 
        response.find("\"status\": true") != std::string::npos) {
        
        std::string expiry = extractJsonString(response, "EXP");
        if (!expiry.empty()) {
            g_ExpTime = expiry;
            g_ServerExpiry = ParseExpiryTime(expiry);
        }
        
        std::string token = extractJsonString(response, "token");
        if (!token.empty()) {
            g_Token = token;
            g_Auth = token;
        } else {
            g_Token = license_key;
            g_Auth = license_key;
        }
        
        logged_in = true;
        bValid = true;
        return true;
    }
    
    std::string reason = extractJsonString(response, "reason");
    ERROR_MESSAGE = !reason.empty() ? reason : "المفتاح غير صالح";
    return false;
}

// ============================================
// دالة تسجيل الدخول
// ============================================
bool Login(std::string androidID, std::string key) {
    is_logging_in = true;
    
    if (androidID.empty()) {
        ERROR_MESSAGE = "لا يمكن الحصول على معرف الجهاز (Android ID)";
        is_logging_in = false;
        return false;
    }
    
    if (key.empty()) {
        ERROR_MESSAGE = "المفتاح فارغ";
        is_logging_in = false;
        return false;
    }
    
    g_ServerExpiry = 0;
    g_ExpTime = "N/A";

    if (verifyWithYourServer(key, androidID)) {
        is_logging_in = false;
        return true;
    }
    
    logged_in = false;
    g_Token = "";
    g_Auth = "";
    bValid = false;
    is_logging_in = false;
    
    return false;
}

#endif // KEYLOGIN_H
