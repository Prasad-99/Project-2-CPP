#ifndef MARKET_H
#define MARKET_H

#include <iostream>
#include <vector>
#include <unordered_map>
#include "Date.h"

using namespace std;

class RateCurve {
public:
  RateCurve(){};
  RateCurve(const string& _name,const Date _asOf): name(_name) ,asOf(_asOf) {} ;
  void addRate(Date tenor, double rate);
  void shock(Date tenor, double value); //implement this
  double getRate(Date tenor) const; //implement this function using linear interpolation
  double getDf(Date _date) const; // using df = exp(-rt), and r is getRate function
  void display() const;

  std::string name;
  Date asOf;//same as market data date

private:
  vector<Date> tenors;
  vector<double> rates; //zero coupon rate or continous compounding rate
};

class VolCurve { // atm vol curve without smile
public:
  VolCurve(){}
  VolCurve(const string& _name,Date _asOf): name(_name) ,asOf(_asOf){} ;
  void addVol(Date tenor, double rate); //implement this
  double getVol(Date tenor) const; //implement this function using linear interpolation
  void display() const; //implement this
  void shock(Date tenor, double value); //implement this

  string name;
  Date asOf;

private:
  vector<Date> tenors;
  vector<double> vols;
};

class Market
{
public:
  Date asOf;
  Market(){
    cout<< "default constructor is called" <<endl;
  };
  Market(const Date& now): asOf(now) {};
  Market(const Market& mkt){
    this->asOf = mkt.asOf;
    RateCurve rateCurve = mkt.curves.begin()->second;
    VolCurve volCurve = mkt.vols.begin()->second;
    addCurve(rateCurve.name,rateCurve);
    addVolCurve(volCurve.name,volCurve);

    for (const auto stockPrice : mkt.stockPrices)
    {
        addStockPrice(stockPrice.first,stockPrice.second);
    }

    for (const auto bondPrice : mkt.bondPrices)
    {
        addBondPrice(bondPrice.first,bondPrice.second);
    }
    cout<<"Cloning Market "<<asOf.toString()
    <<",vols :"<<vols.size()
    <<",curves :"<<curves.size()
    <<",bond :"<<bondPrices.size()
    <<",stock :"<<stockPrices.size()<<endl;

  } //implement this

  void Print() const;
  void addCurve(const std::string& curveName, const RateCurve& curve);//implement this
  void addVolCurve(const std::string& curveName, const VolCurve& curve);//implement this
  void addBondPrice(const std::string& bondName, double price);//implement this
  void addStockPrice(const std::string& stockName, double price);//implement this

  inline void shockPrice(const string& underlying, double shock) { stockPrices[underlying] +=shock; }
  inline RateCurve getCurve(const string& name) { return curves[name]; };
  inline VolCurve getVolCurve(const string& name) { return vols[name]; };

  unordered_map<string, VolCurve> vols;
  unordered_map<string, RateCurve> curves;
  unordered_map<string, double> bondPrices;
  unordered_map<string, double> stockPrices;
};

std::ostream& operator<<(std::ostream& os, const Market& obj);
std::istream& operator>>(std::istream& is, Market& obj);

#endif
