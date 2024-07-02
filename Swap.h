#pragma once
#include "Trade.h"
#include "Date.h"
#include "Market.h"

class Swap : public Trade
{
public:
    // make necessary change
    Swap(std::string name, Date start, Date end, double _notional, double _rate, double _freq) : Trade("SwapTrade", start)
    {
        underlying = name;
        startDate = start;
        maturityDate = end;
        notional = _notional;
        tradeRate = _rate;
        frequency = _freq;
        generateSwapSchedule();
    }

    /*
    implement this, using npv = discounted cash flow from both leg;
    */
    double Payoff(double r) const {};
    double Payoff(double r, const Market &mkt) const;
    double Pv(const Market &mkt) const;
    double getAnnuity(const RateCurve rateCurve) const; // implement this in a cpp file
    void generateSwapSchedule();

private:
    string underlying;
    Date startDate;
    Date maturityDate;
    double notional;
    double tradeRate;
    double frequency; // use 1 for annual, 2 for semi-annual etc
    std::vector<Date> swapSchedule;

    double calculateFixedLegPV() const;
    double calculateFloatingLegPV(const RateCurve rateCurve) const;
};
