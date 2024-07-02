#ifndef _AMERICAN_TRADE
#define _AMERICAN_TRADE

#include <cassert>

#include "TreeProduct.h"
#include "Types.h"
#include "Payoff.h"

class AmericanOption : public TreeProduct {
public:
  AmericanOption(){}
  AmericanOption(OptionType _optType, double _strike,const Date& _startDate, const Date& _expiry): optType(_optType), strike(_strike),startDate(_startDate), expiryDate(_expiry) {}
  virtual double Payoff(double S) const
  {
    return PAYOFF::VanillaOption(optType, strike, S);
  }
  virtual const Date& getExpiry() const
  {
    return expiryDate;
  }

  virtual const Date& getStartDate() const { return startDate; }
  virtual double ValueAtNode(double S, double t, double continuation) const
  {
    return std::max(Payoff(S), continuation);
  }

private:
  OptionType optType;
  double strike;
  Date expiryDate;
  Date startDate;
};

class AmerCallSpread : public TreeProduct {
public:
    AmerCallSpread(double _k1, double _k2,const Date& _startDate, const Date& _expiry)
    : strike1(_k1), strike2(_k2), expiryDate(_expiry) , startDate(_startDate)
    {
      assert(_k1 < _k2);
    };
    virtual double Payoff(double S) const
    {
      return PAYOFF::CallSpread(strike1, strike2, S);
    }
    virtual const Date& getExpiry() const
    {
      return expiryDate;
    }
    virtual const Date& getStartDate() const { return startDate; };

private:
  double strike1;
  double strike2;
  Date expiryDate;
  Date startDate;
};

#endif
