#pragma once
#include "../Theme.h"
class RangeSelector:public juce::ComboBox {
public:RangeSelector(){addItemList({"I","II","III"},1);setTooltip("Select body (I), upper mids (II), or presence (III).");}
};
