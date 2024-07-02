#include "RiskEngine.h"


void RiskEngine::computeRisk(string riskType, std::shared_ptr<Trade> trade, bool singleThread) 
{
    result.clear();
    if (singleThread) {
        if(riskType == "dv01"){
            for (auto& kv: curveShocks) {
                string market_id = kv.first;
                auto mkt_u = kv.second.getMarketUp();
                auto mkt_d = kv.second.getMarketDown();
                
                double pv_up = trade->Pv(mkt_u);
                double pv_down = trade->Pv(mkt_d);
                double dv01 = (pv_up - pv_down)/2.0;
                result.emplace(market_id, dv01);
            }
        }
        if (riskType == "vega") {
            for (auto& kv : volShocks) {
                string market_id = kv.first;
                auto vol_up = kv.second.getVolMarketUp();
                auto vol_down = kv.second.getVolMarketDown();

                double vol_pv_up = trade->Pv(vol_up);
                double vol_pv_down = trade->Pv(vol_down);

                double vega = (vol_pv_up - vol_pv_down)/2.0;

                result.emplace(market_id, vega);
            }
        }

        if (riskType == "price") {
            // to be checked.
            for (auto& kv : priceShocks) {
                string market_id = kv.first;
                auto price_after_shock = kv.second.getMarket();

                double price_pv_after_shock = trade->Pv(price_after_shock);

                result.emplace(market_id, price_pv_after_shock);
            }
        }
    }
    else {
        auto pv_task = [](shared_ptr<Trade> trade, string id, const Market& mkt_up, const Market& mkt_down) {
            double pv_up = trade->Pv(mkt_up);
            double pv_down = trade->Pv(mkt_down);
            double dv01 = (pv_up - pv_down)/2.0;
            return std::make_pair(id, dv01);
        };

        vector<std::future<std::pair<string, double>>> _futures;
        // calling the above function asynchronously and storing the result in future object
        for (auto& shock: curveShocks) {
            string market_id = shock.first;
            auto mkt_u = shock.second.getMarketUp();
            auto mkt_d = shock.second.getMarketDown();
            _futures.push_back(std::async(std::launch::async, pv_task, trade, market_id, mkt_u, mkt_u)); 
        }
        
        for (auto && fut: _futures) {
            auto rs = fut.get();
            result.emplace(rs);
        }     

    }
}