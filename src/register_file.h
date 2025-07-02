#ifndef REGISTER_FILE_H
#define REGISTER_FILE_H


// register_file.h
#pragma once
#include "comm.h"

class RegisterFile {
public:
    /**
     * @brief سازنده کلاس که تمام رجیسترها را صفر می‌کند.
     * مطابق با مستندات، در ابتدای راه‌اندازی شبیه‌ساز، مقدار تمامی ثبات‌ها صفر است.
     */
    RegisterFile();

    /**
     * @brief نوشتن یک مقدار ۳۲ بیتی در رجیستر مشخص شده.
     * @param reg_index شماره رجیستر (0 تا 31).
     * @param value مقداری که باید نوشته شود.
     * این تابع بررسی می‌کند که در رجیستر x0 (صفر) چیزی نوشته نشود.
     */
    void write(uint8_t reg_index, uint32_t value);

    /**
     * @brief خواندن مقدار ۳۲ بیتی از یک رجیستر.
     * @param reg_index شماره رجیستر (0 تا 31).
     * @return مقدار ذخیره شده در رجیستر.
     */
    uint32_t read(uint8_t reg_index) const;

    /**
     * @brief تابعی کمکی برای نمایش محتوای تمام رجیسترها (برای دیباگ).
     */
    void dump() const;
    std::array<uint32_t, 32> get_all_registers() const;

private:
    // آرایه‌ای با اندازه ثابت برای نگهداری ۳۲ رجیستر ۳۲ بیتی.
    std::array<uint32_t, 32> registers;
};



#endif //REGISTER_FILE
