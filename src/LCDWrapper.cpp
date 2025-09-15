#include <LiquidCrystal.h>

class LCDWrapper
{
private:
    LiquidCrystal &lcd;
    bool flipped;

public:
    LCDWrapper(LiquidCrystal &lcdRef, bool flipped = false) : lcd(lcdRef), flipped(flipped) {}

    void setFlipped(bool state) { flipped = state; }

    template <typename T>
    void print(T value) {
        if constexpr (std::is_same_v<T, char>)
    }
};