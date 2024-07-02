#include "Swap.h"
#include "Market.h"
#include "cmath"

void Swap::generateSwapSchedule()
{
  if (startDate == maturityDate || frequency <= 0 || frequency > 1)
    throw std::runtime_error("Error: start date is later than end date, or invalid frequency!");

  string tenorStr;
  if (frequency == 0.25)
    tenorStr = "3M";
  else if (frequency == 0.5)
    tenorStr = "6M";
  else
    tenorStr = "1Y";

  Date seed = startDate;
  while (seed < maturityDate)
  {
    swapSchedule.push_back(seed);
    Date newSeed = dateAddTenor(seed, tenorStr);
    seed = newSeed;
  }
  swapSchedule.push_back(maturityDate);
  if (swapSchedule.size() < 2)
    throw std::runtime_error("Error: invalid schedule, check input!");
}

double Swap::Payoff(double s, const Market &mkt) const
{
  // this is using annutiy to compute pv
  auto thisMkt = const_cast<Market &>(mkt);
  auto rateCurve = thisMkt.getCurve("usd-sofr");
  return (s - tradeRate) * getAnnuity(rateCurve);
}

// Method to calculate present value of fixed leg
double Swap::calculateFixedLegPV() const
{
  double fixedPV = 0.0;
  for (int i = 1; i <= swapSchedule.size(); ++i)
  {
    double payment = notional * tradeRate / frequency;
    double discountFactor = pow(1.0 + tradeRate / frequency, -i);
    fixedPV += payment * discountFactor;
  }
  return fixedPV;
}

// Method to calculate present value of floating leg
double Swap::calculateFloatingLegPV(const RateCurve rateCurve) const
{
  double floatingPV = 0.0;
  for (int i = 1; i <= swapSchedule.size(); ++i)
  {
    double floatingRate = rateCurve.getRate(swapSchedule[i]);
    double payment = notional * floatingRate / frequency;
    double discountFactor = pow(1.0 + tradeRate / frequency, -i);
    floatingPV += payment * discountFactor;
  }
  return floatingPV;
}

double Swap::getAnnuity(const RateCurve rateCurve) const
{
  int numPeriods = static_cast<int>(frequency * yearsBetween(startDate, maturityDate)); // Calculate number of periods

  // Calculate PV of fixed leg
  double fixedPV = calculateFixedLegPV();

  // Calculate PV of floating leg
  double floatingPV = calculateFloatingLegPV(rateCurve);

  // Calculate NPV of swap
  double npvSwap = floatingPV - fixedPV;

  // Calculate annuity payment using the NPV of the swap
  double annuityPayment = npvSwap * (tradeRate / (1 - pow(1 + tradeRate, -numPeriods)));

  return annuityPayment;
}

double Swap::Pv(const Market &mkt) const
{
  // using cash flow discunting
  auto thisMkt = const_cast<Market &>(mkt);
  double fixPv = 0;

  auto rateCurve = thisMkt.getCurve("usd-sofr");
  double df = rateCurve.getDf(maturityDate);
  double fltPv = (-notional + notional * df);
  for (auto &dt : swapSchedule)
  {
    if (dt == startDate)
      continue;
    double tau = dt - startDate;
    df = rateCurve.getDf(dt);
    fixPv += notional * tau * tradeRate * df;
  }

  return fixPv + fltPv;
}
