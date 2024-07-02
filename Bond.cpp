#include "Bond.h"
#include "Market.h"
#include <cmath>

void Bond::generateBondSchedule()
{
  // implement this
  // for this project, assume bond pays either annual, semi-annual or quarterly.
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
    bondSchedule.push_back(seed);
    Date newSeed = dateAddTenor(seed, tenorStr);
    seed = newSeed;
  }
  bondSchedule.push_back(maturityDate);
  if (bondSchedule.size() < 2)
    throw std::runtime_error("Error: invalid schedule, check input!");
}

double Bond::Payoff(double s) const
{
  double pv = notional * (s - tradePrice);
  return pv;
}

double Bond::Pv(const Market &mkt) const
{
  // using cash flow discunting
  //  implement this
  auto thisMkt = const_cast<Market &>(mkt);
  double cashflowPv = 0;
  double interest = notional * coupon * frequency;
  // cout << "interest: " << interest << endl;
  // cout << "coupon: " << coupon << endl;
  // cout << "frequency: " << frequency << endl;
  auto rateCurve = thisMkt.getCurve("usd-sofr");
  double df = rateCurve.getDf(maturityDate);
  for (auto &dt : bondSchedule)
  {
    if (dt == startDate)
      continue;
    double tau = dt - startDate;
    df = rateCurve.getDf(dt);
    cashflowPv += interest * (pow(df, tau/frequency)) ;
    // cout << "fixPv: " << cashflowPv << " tau: " << tau << " df: " << df << " dt: " << dt << endl;
  }

  double bondPv = cashflowPv + notional * pow(df, (maturityDate - startDate)/frequency);
  // cout << "fix bondPv: " << bondPv << endl;
  return bondPv;
}
