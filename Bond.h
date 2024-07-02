#pragma once
#include "Trade.h"

class Bond : public Trade {
public:
    Bond(std::string name, Date start, Date end, double _notional, double rate, double freq): Trade("BondTrade", start) {
        underlying = name;
        notional = _notional;
        startDate = start;
        maturityDate = end;
        frequency = freq;
        coupon = rate;
        generateBondSchedule();
    }
    double Payoff(double s) const; // implement this
    double Pv(const Market& mkt) const; // implement this
    void generateBondSchedule(); //implement this

private:
    std::string underlying;
    double notional;
    double tradePrice;
    double coupon;
    double frequency;

    Date startDate;
    Date maturityDate;
    std::vector<Date> bondSchedule;

};
