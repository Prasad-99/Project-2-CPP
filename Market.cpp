#include "Market.h"
#include <cmath>

using namespace std;

void RateCurve::display() const
{
  cout << "rate curve:" << name << endl;
  for (size_t i = 0; i < tenors.size(); i++)
  {
    cout << tenors[i].toString() << ": " << rates[i] << endl;
  }
  cout << endl;
}

void RateCurve::addRate(Date tenor, double rate)
{
  // consider to check if tenor already exist
  if (std::find(tenors.begin(), tenors.end(), tenor) == tenors.end())
  {
    tenors.push_back(tenor);
    rates.push_back(rate);
  }
}

void RateCurve::shock(Date tenor, double value)
{
  for(int x : rates)
    x += value;
}

double RateCurve::getRate(Date tenor) const
{
  // use linear interpolation to get rate
  int tenorSize = tenors.size();

  if (tenorSize == 0)
  {
    cout << "fix Error: no tenor in the rate curve" << endl;
    return 0;
  }

  int i = 0;                          // find left end of interval for interpolation
  if (tenor >= tenors[tenorSize - 2]) // special case: beyond right end
  {
    i = tenorSize - 2;
  }
  else
  {
    while (tenor > tenors[i + 1])
      i++;
  }
  Date xL = tenors[i];
  double yL = rates[i];
  Date xR = tenors[i + 1];
  double yR = rates[i + 1];

  double dydx = (yR - yL) / (xR - xL); // gradient
  return yL + dydx * (tenor - xL); // linear interpolation
  // return 0;
}

double RateCurve::getDf(Date _date) const
{
  double ccr = getRate(_date);
  double t = _date - asOf;
  return exp(-ccr * t);
}

void VolCurve::display() const
{
  cout << "Vol curve:" << name << endl;
  for (size_t i = 0; i < tenors.size(); i++)
  {

    cout << tenors[i].toString() << ": " << vols[i] << endl;
  }
  cout << endl;
}

void VolCurve::addVol(Date tenor, double rate)
{
  // consider to check if tenor already exist

  if (std::find(tenors.begin(), tenors.end(), tenor) == tenors.end())
  {
    tenors.push_back(tenor);
    vols.push_back(rate);
  }
}

void VolCurve::shock(Date tenor, double value)
{
  for(int x : vols)
    x += value;
}

double VolCurve::getVol(Date x) const
{
    int size = tenors.size();

    int i = 0;                 // find left end of interval for interpolation
    if (x >= tenors[size - 2]) // special case: beyond right end
    {
        i = size - 2;
    }
    else
    {
        while (x > tenors[i + 1])
            i++;
    }
    Date xL = tenors[i];
    double yL = vols[i];
    Date xR = tenors[i + 1];
    double yR = vols[i + 1]; // points on either side (unless beyond ends)

    double dydx = (yR - yL) / (xR - xL); // gradient

    return yL + dydx * (x - xL); // linear interpolation
}

void Market::Print() const
{

  cout << "-------Market asof: " << asOf.toString() <<"-------"<< endl;
  cout << "--Rate Curve-- " << endl;
  for (auto curve : curves)
  {
  std:
    cout << curve.first << endl;
    curve.second.display();
  }

  cout << "--Vol Curve-- " << endl;
  for (auto vol : vols)
  {
    cout << vol.first << endl;
    vol.second.display();
  }
  /*
  add display for bond price and stock price

  */

   cout << "--Stock-- " << endl;
   for (const auto stockPrice : stockPrices)
  {
   cout<<stockPrice.first<<":"<< stockPrice.second<<endl;

  }


  cout << "--Bond-- "  << endl;
  for (const auto bondPrice : bondPrices)
  {
    cout <<bondPrice.first<<":"<< bondPrice.second<<endl;
  }

  cout<<endl;

}

void Market::addCurve(const std::string &curveName, const RateCurve &curve)
{
  curves.emplace(curveName, curve);
}

void Market::addVolCurve(const std::string &curveName, const VolCurve &curve)
{
  vols.emplace(curveName, curve);
}

void Market::addBondPrice(const std::string &bondName, double price)
{
  bondPrices.emplace(bondName, price);
}

void Market::addStockPrice(const std::string &stockName, double price)
{
  stockPrices.emplace(stockName, price);
}

std::ostream &operator<<(std::ostream &os, const Market &mkt)
{
  os << mkt.asOf << std::endl;
  return os;
}

std::istream &operator>>(std::istream &is, Market &mkt)
{
  is >> mkt.asOf;
  return is;
}
