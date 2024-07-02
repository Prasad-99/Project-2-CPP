#pragma once
#include <iostream>
#include <string>
#include <map>
#include <unordered_map>
#include <future>

#include "Trade.h"
#include "Market.h"

using namespace std;

struct MarketShock {
    string market_id;
    pair<Date, double> shock; //tenor and value
};

class CurveDecorator : public Market {
public:
	CurveDecorator(const Market& mkt, const MarketShock& curveShock) : thisMarketUp(mkt), thisMarketDown(mkt)
    {
        cout<< "curve decorator is created"<<endl;
        auto curve_up = thisMarketUp.getCurve(curveShock.market_id);
        curve_up.shock(curveShock.shock.first, curveShock.shock.second);
        cout<< "curve tenor " << curveShock.shock.first << "is shocked " << curveShock.shock.second<<endl;

        auto curve_down = thisMarketDown.getCurve(curveShock.market_id);
        curve_down.shock(curveShock.shock.first, -1*curveShock.shock.second);
        cout<< "curve tenor " << curveShock.shock.first << "is shocked " << curveShock.shock.second<<endl;

    }
    inline const Market getMarketUp() const { return thisMarketUp;}
    inline const Market getMarketDown() const { return thisMarketDown;}

private:
	Market thisMarketUp;
    Market thisMarketDown;
};

class VolDecorator : public Market {
public:
	VolDecorator(const Market& mkt, const MarketShock& volShock) : thisVolMarketUp(mkt), thisVolMarketDown(mkt)
    {
        cout<< "vol decorator is created"<<endl;
        auto vol_curve_up = thisVolMarketUp.getVolCurve(volShock.market_id);
        vol_curve_up.shock(volShock.shock.first, volShock.shock.second);
        cout<< "vol tenor " << volShock.shock.first << "is shocked" << volShock.shock.second<<endl;

        auto vol_curve_down = thisVolMarketDown.getVolCurve(volShock.market_id);
        vol_curve_down.shock(volShock.shock.first, -1*volShock.shock.second);
        cout<< "curve tenor " << volShock.shock.first << "is shocked " << volShock.shock.second<<endl;
    }

    inline const Market& getVolMarketUp() const { return thisVolMarketUp;}
    inline const Market& getVolMarketDown() const { return thisVolMarketDown;}

private:
	Market thisVolMarketUp;
    Market thisVolMarketDown;
};

class PriceDecorator : public Market {
public:
	PriceDecorator(const Market& mkt, const MarketShock& priceShock) : thisMarket(mkt)
    {
        cout<< "stock price decorator is created"<<endl;
        thisMarket.shockPrice(priceShock.market_id, priceShock.shock.second);
    }

    inline const Market& getMarket() const { return thisMarket;}

private:
	Market thisMarket;
};

class RiskEngine
{
public:

    RiskEngine(const Market& market, double curve_shock, double vol_shock, double price_shock) {
        //add implementation, create curve shocks, vol shocks w.r.t to curve structure etc
        cout << " risk engine is created .. " << endl;
        market.Print();
        curveShocks.emplace("curve", CurveDecorator(market, {"usd-sofr", {Date(2024, 6, 1), curve_shock}}));
        // volShocks.emplace("vol", VolDecorator(market, {"Vol_VD1", {Date(2024, 6, 1), vol_shock}}));
        // priceShocks.emplace("price", PriceDecorator(market, {"stock1", {Date(2024, 6, 1), price_shock}}));
    };

    void computeRisk(string riskType, std::shared_ptr<Trade> trade, bool singleThread);

    inline map<string, double> getResult() const {
        cout << " risk result: " << endl;
        return result;
        };


private:
    unordered_map<string, CurveDecorator> curveShocks; //tenor, shock
    unordered_map<string, VolDecorator> volShocks;
    unordered_map<string, PriceDecorator> priceShocks;

    map<string, double> result;

};

