#include "ui.h"

#include "main.h"
#include "lang.h"
#include "bitmap.h"
#include "util.h"

template <>
void UIImpl<UISize::LG>::loading(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2) {
    startDraw(epd);
    u8g2.setFont(u8g2_font_wqy14_t);
    drawCenteredString(u8g2, epd.width() / 2, epd.height() * 2 / 3, TEXT_LOADING);
    endDraw(epd, true);
}

template <>
void UIImpl<UISize::LG>::smartConfig(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2) {
    startDraw(epd);
    drawQRCode(epd, (epd.width() - 116) / 2, 16, 4, WIFI_CONFIG_URL);
    u8g2.setFont(u8g2_font_wqy14_t);
    drawCenteredString(u8g2, epd.width() / 2, epd.height() - 16, TEXT_SMART_CONFIG);
    endDraw(epd, true);
}

template <>
void UIImpl<UISize::LG>::syncTimeFailed(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2) {
    startDraw(epd);
    u8g2.setFont(u8g2_font_wqy14_t);
    u8g2.setForegroundColor(COLOR_ERROR);
    drawCenteredString(u8g2, epd.width() / 2, epd.height() * 2 / 3, TEXT_TIME_ERROR);
    u8g2.setForegroundColor(COLOR_PRIMARY);
    endDraw(epd, true);
}

template <>
void UIImpl<UISize::LG>::lowPower(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2) {
    startDraw(epd);
    u8g2.setFont(u8g2_font_wqy14_t);
    drawCenteredString(u8g2, epd.width() / 2, epd.height() * 2 / 3, TEXT_LOW_POWER);
    endDraw(epd);
}

template <>
void UIImpl<UISize::LG>::update(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2) {
    startDraw(epd);
    epd.fillTriangle(epd.width() / 2, 20, epd.width() / 2 - 24, 44, epd.width() / 2 + 24, 44, COLOR_PRIMARY);
    epd.fillRect(epd.width() / 2 - 12, 44, 24, 42, COLOR_PRIMARY);
    u8g2.setFont(u8g2_font_wqy14_t);
    drawCenteredString(u8g2, epd.width() / 2, epd.height() - 36, TEXT_UPDATING_1);
    drawCenteredString(u8g2, epd.width() / 2, epd.height() - 16, TEXT_UPDATING_2);
    endDraw(epd);
}

template <>
void UIImpl<UISize::LG>::titleBar(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2, tm *ptime, bool sleeping, int8_t rssi, int8_t battery) {
    String title = datetimeToString(FORMAT_DATETIME, ptime) + " 更新";
    startDraw(epd);
    drawTitleBar(epd, u8g2, title.c_str(), sleeping, rssi, battery);
    epd.displayWindow(0, 0, epd.width(), 16);
    epd.hibernate();
}

static void drawWeatherNow(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2, uint16_t y, Weather &weather, DailyWeather &today, AQI &aqi) {
    // Draw weather icon and text
    u8g2.setFont(u8g2_font_qweather_icon_16);
    u8g2.drawUTF8(8, y + 40, getWeatherIcon(weather.icon, isNight(weather.time)));
    u8g2.setFont(u8g2_font_wqy14_t);
    u8g2.setCursor(40, y + 16);
    u8g2.printf("%s  %d°C", weather.text.c_str(), weather.temp);
    u8g2.setCursor(40, y + 32);
    u8g2.printf("%d ~ %d°C", today.tempMin, today.tempMax);
    u8g2.setCursor(40, y + 48);
    u8g2.printf("体感 %d°C", weather.feelsTemp);
    // Draw separator
    epd.drawFastVLine(epd.width() / 4 + 8, y, 54, COLOR_PRIMARY);
    // Draw weather details
    u8g2.setFont(u8g2_font_wqy14_t);
    u8g2.setCursor(epd.width() / 4 + 24, y + 16);
    u8g2.printf("%s %s级  湿度: %d%%", weather.windDir.c_str(), weather.windScale.c_str(), weather.humidity);
    u8g2.setCursor(epd.width() / 4 + 24, y + 32);
    u8g2.printf("气压: %d  能见度: %d KM", weather.pressure, weather.visibility);
    u8g2.setCursor(epd.width() / 4 + 24, y + 48);
    u8g2.printf("紫外线: %d  %s %d", today.uvIndex, aqi.category.c_str(), aqi.aqi);
    // Draw separator
    epd.drawFastVLine(epd.width() * 3 / 4 - 8, y, 54, COLOR_PRIMARY);
    // Draw sun and moon info
    u8g2.setCursor(epd.width() * 3 / 4, y + 16);
    u8g2.printf("日出%s", today.sunrise.c_str());
    u8g2.setCursor(epd.width() * 3 / 4, y + 32);
    u8g2.printf("日落%s", today.sunset.c_str());
    drawCenteredString(u8g2, epd.width() * 3 / 4 + 32, y + 48, today.moonPhase.c_str());
    u8g2.setFont(u8g2_font_qweather_icon_16);
    const char *icon = getWeatherIcon(today.moonPhaseIcon);
    u8g2.drawUTF8(epd.width() - u8g2.getUTF8Width(icon) - 8, y + 40, icon);
}

static void drawForecastHourly(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2, uint16_t y, uint16_t h, HourlyForecast &forecast) {
    uint16_t grid_w = epd.width() / forecast.length;
    int8_t min_temp = forecast.weather[0].temp;
    int8_t max_temp = forecast.weather[0].temp;
    // Draw hourly forecast
    u8g2.setFont(u8g2_font_wqy14_t);
    for (uint8_t i = 0; i < forecast.length; i++) {
        Weather &weather = forecast.weather[i];
        if (weather.temp < min_temp) {
            min_temp = weather.temp;
        }
        if (weather.temp > max_temp) {
            max_temp = weather.temp;
        }
        uint16_t x = grid_w * i + grid_w / 2;
        drawCenteredString(u8g2, x, y + 14, weather.time.substring(11, 16).c_str());
        u8g2.setFont(u8g2_font_qweather_icon_16);
        drawCenteredString(u8g2, x, y + 42, getWeatherIcon(weather.icon, isNight(weather.time)));
        u8g2.setFont(u8g2_font_wqy14_t);
        String temp = String(weather.temp) + "°C";
        drawCenteredString(u8g2, x, y + 56, temp.c_str());
    }
#if SHOW_WEATHER_HOURLY_CURVE == true
    // Draw temperature curve
    uint16_t prev_x = 0;
    uint16_t prev_y = 0;
    for (uint8_t i = 0; i < forecast.length; i++) {
        Weather &weather = forecast.weather[i];
        uint16_t point_x = grid_w * i + grid_w / 2;
        uint16_t point_y = (uint16_t) map(weather.temp, min_temp, max_temp, y + h - 12, y + 56 + 12);
        epd.drawCircle(point_x, point_y, 4, COLOR_PRIMARY);
        if (i > 0) {
            epd.drawLine(prev_x, prev_y, point_x, point_y, COLOR_PRIMARY);
        }
        prev_x = point_x;
        prev_y = point_y;
    }
#endif
}

static void drawForecastDaily(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2, uint16_t y, uint16_t h, int dayOfWeek, DailyForecast &forecast) {
    uint16_t grid_w = epd.width() / forecast.length;
    int8_t min_temp = forecast.weather[0].tempMin;
    int8_t max_temp = forecast.weather[0].tempMax;
    // Draw daily forecast
    u8g2.setFont(u8g2_font_wqy14_t);
    for (uint8_t i = 0; i < forecast.length; i++) {
        DailyWeather &weather = forecast.weather[i];
        if (weather.tempMin < min_temp) {
            min_temp = weather.tempMin;
        }
        if (weather.tempMax > max_temp) {
            max_temp = weather.tempMax;
        }
        uint16_t x = grid_w * i + grid_w / 2;
        const char *dayText = nullptr;
        if (i == 0) {
            dayText = "今天";
        } else if (i == 1) {
            dayText = "明天";
        } else {
            dayText = WEEKDAYS[(dayOfWeek + i) % 7];
        }
        drawCenteredString(u8g2, x, y + 18, dayText);
        String date = weather.date.substring(5, 10);
        drawCenteredString(u8g2, x, y + 32, date.c_str());
        u8g2.setFont(u8g2_font_qweather_icon_16);
        drawCenteredString(u8g2, x, y + 60, getWeatherIcon(weather.iconDay, isNight(weather.date)));
        u8g2.setFont(u8g2_font_wqy14_t);
        String temp = String(weather.tempMin) + "~" + String(weather.tempMax) + "°C";
        drawCenteredString(u8g2, x, y + h - 18, temp.c_str());
        String wind = String(weather.windScaleDay) + "级";
        drawCenteredString(u8g2, x, y + h - 4, wind.c_str());
    }
#if SHOW_WEATHER_DAILY_CURVE == true
    // Draw temperature curve
    uint16_t prev_x = 0;
    uint16_t prev_y1 = 0;
    uint16_t prev_y2 = 0;
    for (uint8_t i = 0; i < forecast.length; i++) {
        DailyWeather &weather = forecast.weather[i];
        uint16_t point_x = grid_w * i + grid_w / 2;
        uint16_t point_y1 = (uint16_t) map(weather.tempMin, min_temp, max_temp, y + h - 32 - 8, y + 60 + 8);
        uint16_t point_y2 = (uint16_t) map(weather.tempMax, min_temp, max_temp, y + h - 32 - 8, y + 60 + 8);
        epd.drawCircle(point_x, point_y1, 2, COLOR_PRIMARY);
        epd.drawCircle(point_x, point_y2, 2, COLOR_PRIMARY);
        if (i > 0) {
            epd.drawLine(prev_x, prev_y1, point_x, point_y1, COLOR_PRIMARY);
            epd.drawLine(prev_x, prev_y2, point_x, point_y2, COLOR_PRIMARY);
        }
        prev_x = point_x;
        prev_y1 = point_y1;
        prev_y2 = point_y2;
    }
#endif
}

template <>
void UIImpl<UISize::LG>::weather(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2, tm *ptime, bool sleeping, int8_t rssi, int8_t battery) {
    String title = datetimeToString(FORMAT_DATETIME, ptime) + " 更新";
    Weather currentWeather = {};
    Weather hourlyWeather[8] = {};
    DailyWeather dailyWeather[7] = {};
    AQI aqi = {};
    HourlyForecast hourlyForecast = {
        .weather = hourlyWeather,
        .length = ARRAY_LENGTH(hourlyWeather),
        .interval = config.hour_step
    };
    DailyForecast dailyForecast = {
        .weather = dailyWeather,
        .length = ARRAY_LENGTH(dailyWeather)
    };
    bool success = api.getWeatherNow(currentWeather, config.location_id)
                    & api.getForecastHourly(hourlyForecast, config.location_id)
                    & api.getForecastDaily(dailyForecast, config.location_id)
                    & api.getAQI(aqi, config.location_id);

    startDraw(epd);
    drawTitleBar(epd, u8g2, title.c_str(), sleeping, rssi, battery);
    uint16_t y = 16;
    drawWeatherNow(epd, u8g2, y, currentWeather, dailyWeather[0], aqi);
    y += 54;
#if SHOW_WEATHER_HOURLY_CURVE == true && SHOW_WEATHER_DAILY_CURVE == false
    uint16_t h = (epd.height() - y) - 100;
#elif SHOW_WEATHER_HOURLY_CURVE == false && SHOW_WEATHER_DAILY_CURVE == true
    uint16_t h = 60;
#else
    uint16_t h = (epd.height() - y) * 2 / 5;
#endif
    epd.drawFastHLine(0, y, epd.width(), COLOR_PRIMARY);
    drawForecastHourly(epd, u8g2, y, h, hourlyForecast);
    y += h;
    h = epd.height() - y;
    epd.drawFastHLine(0, y, epd.width(), COLOR_PRIMARY);
    drawForecastDaily(epd, u8g2, y, h, ptime->tm_wday, dailyForecast);
    if (!success) {
        u8g2.setFont(u8g2_font_wqy14_t);
        u8g2.setForegroundColor(COLOR_ERROR);
        drawCenteredString(u8g2, epd.width() / 2, epd.height() / 2 + 8, TEXT_WEATHER_FAILED);
        u8g2.setForegroundColor(COLOR_PRIMARY);
    }
    endDraw(epd);
}

template <>
void UIImpl<UISize::LG>::bilibili(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2) {
    Bilibili bilibili = {};
    api.getFollower(bilibili, config.bilibili_uid);
    bool success = false;
    if (*config.bilibili_cookie != '\0') {
        success = api.getLikes(bilibili, config.bilibili_uid, config.bilibili_cookie);
    }

    startDraw(epd);
    epd.drawInvertedBitmap(16, (epd.height() - 65) / 2, IMG_BILI_LOGO_2, 68, 65, COLOR_PRIMARY);
    uint16_t x = 100;
    uint16_t cx = (x + epd.width()) / 2;
    uint16_t cy = epd.height() / 2;
    u8g2.setFont(u8g2_font_fub30_tf);
    drawCenteredString(u8g2, cx, success ? cy - 32 : cy + 15, humanizeNumber(bilibili.follower).c_str());
    if (success) {
        u8g2.setFont(u8g2_font_bili_icon_16);
        u8g2.drawUTF8(x + 10, cy + 48, "\uE6E3");
        u8g2.drawUTF8(x + 68, cy + 48, "\uE6E0");
        u8g2.setFont(u8g2_font_wqy14_t);
        u8g2.drawUTF8(cx + 10, cy + 48, humanizeNumber(bilibili.view).c_str());
        u8g2.drawUTF8(cx + 70, cy + 48, humanizeNumber(bilibili.likes).c_str());
    }
    endDraw(epd);
}

template <>
void UIImpl<UISize::LG>::display(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2, const String &ip, const String &text) {
    startDraw(epd);
    u8g2.setFont(u8g2_font_wqy14_t);
    u8g2.setCursor(0, 16);
    if (text.length() > 0) {
        u8g2.print(text);
    } else {
        u8g2.printf(TEXT_CUSTOM_EMPTY, ip.c_str());
    }
    endDraw(epd);
}

template <>
void UIImpl<UISize::LG>::about(EPD_CLASS &epd, U8G2_FOR_ADAFRUIT_GFX &u8g2, const String &ip) {
    Hitokoto hitokoto = {};
    api.getHitokoto(hitokoto);

    startDraw(epd);
    u8g2.setFont(u8g2_font_wqy14_t);
    u8g2.setCursor(0, 64);
    u8g2.printf("    墨水屏智能助理 %s\n", version);
    u8g2.println("    Copyright (C) 2022-2025 WC");
    u8g2.println("    气象数据由 和风天气 提供");
    u8g2.println("");
    u8g2.println("    IP: " + ip);
    u8g2.println("");
    u8g2.print("    ");
    u8g2.println(hitokoto.sentence);
    endDraw(epd);
}
