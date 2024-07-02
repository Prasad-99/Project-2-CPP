#include <fstream>
#include <ctime>
#include <chrono>
#include <memory>
#include <iomanip>

#include "Market.h"
#include "Pricer.h"
#include "RiskEngine.h"
#include "Factory.h"
#include "thread_pool.h"
#include "ConfigManager.h"
#include "Constants.h"

using namespace std;

/**
 * @brief Main function to run the market data update, portfolio creation, pricing, and risk calculation tasks.
 * @return int Exit status of the program.
 */
int main()
{
    // Task 1: Create a market data object and update the market data from txt files (load 2 days market data)

    std::shared_ptr<ConfigManager> configManager(ConfigManager::GetInstance());

    std::string const configDirectory = "config/";
    std::string const configFileName = "config.txt";
    configManager->loadConfig(configDirectory + configFileName);

    std::string marketDataDirectory = configManager->getConfig(MD_FOLDER_PATH);
    std::string valueDateString = configManager->getConfig(VD);

    Date initValueDate;
    std::vector<std::string> res = split(valueDateString, DATE_DELIMITER);

    if (res.size() != 3)
    {
        throw std::invalid_argument("Invalid date initialised: " + valueDateString);
    }

    initValueDate.year = std::stoi(res[0]);
    initValueDate.month = std::stoi(res[1]);
    initValueDate.day = std::stoi(res[2]);

    cout << "--------------------Init Value Date-------------------------" << endl;

    Date valueDate = initValueDate;
    Date valueDatePlusOne = initValueDate;
    valueDatePlusOne.day += 1;

    cout << "Value Date: " << valueDate.toString() << endl;
    cout << "Value Date + 1: " << valueDatePlusOne.toString() << endl;
    cout << "-------------------------------------------------------------" << endl;
    cout << endl;

    Market mktVD0(valueDate);
    Market mktVD1(valueDatePlusOne);

    // Load stock price, USD SOFR, vol data

    std::ifstream stockPriceDay1(marketDataDirectory + "stock_price_20240601.txt");
    std::ifstream stockPriceDay2(marketDataDirectory + "stock_price_20240602.txt");

    std::ifstream usdSofrDay1(marketDataDirectory + "usd_sofr_20240601.txt");
    std::ifstream usdSofrDay2(marketDataDirectory + "usd_sofr_20240602.txt");

    std::ifstream volDay1(marketDataDirectory + "vol_20240601.txt");
    std::ifstream volDay2(marketDataDirectory + "vol_20240602.txt");

    std::ifstream portfolio(marketDataDirectory + "portfolio.txt");

    // Load USD SOFR data for day 1 and day 2
    RateCurve rateCurveDay1(USD_SOFR, valueDate);
    RateCurve rateCurveDay2(USD_SOFR, valueDatePlusOne);

    std::string fileLine = "";

    while (getline(usdSofrDay1, fileLine))
    {
        string tenor = fileLine.substr(0, fileLine.find(COLON_DELIMITER));
        string rate = fileLine.substr(fileLine.find(COLON_DELIMITER) + COLON_DELIMITER.length());
        Date tenorDate = dateAddTenor(valueDate, tenor);
        rateCurveDay1.addRate(tenorDate, stod(rate) / 100);
    }

    while (getline(usdSofrDay2, fileLine))
    {
        string tenor = fileLine.substr(0, fileLine.find(COLON_DELIMITER));
        string rate = fileLine.substr(fileLine.find(COLON_DELIMITER) + COLON_DELIMITER.length());
        Date tenorDate = dateAddTenor(valueDatePlusOne, tenor);
        rateCurveDay2.addRate(tenorDate, stod(rate) / 100);
    }

    // Load volatility data for day 1 and day 2
    VolCurve volCurveDay1("Vol_VD1", valueDate);
    VolCurve volCurveDay2("Vol_VD2", valueDatePlusOne);

    while (getline(volDay1, fileLine))
    {
        string tenor = fileLine.substr(0, fileLine.find(COLON_DELIMITER));
        string vol = fileLine.substr(fileLine.find(COLON_DELIMITER) + COLON_DELIMITER.length());
        Date tenorDate = dateAddTenor(valueDate, tenor);
        volCurveDay1.addVol(tenorDate, stod(vol) / 100);
    }

    while (getline(volDay2, fileLine))
    {
        string tenor = fileLine.substr(0, fileLine.find(COLON_DELIMITER));
        string vol = fileLine.substr(fileLine.find(COLON_DELIMITER) + COLON_DELIMITER.length());
        Date tenorDate = dateAddTenor(valueDatePlusOne, tenor);
        volCurveDay2.addVol(tenorDate, stod(vol) / 100);
    }

    mktVD0.addCurve("usd_sofr_day1", rateCurveDay1);
    mktVD1.addCurve("usd_sofr_day2", rateCurveDay2);
    mktVD0.addVolCurve("vol_day1", volCurveDay1);
    mktVD1.addVolCurve("vol_day2", volCurveDay2);

    // Load stock prices for day 1 and day 2
    while (getline(stockPriceDay1, fileLine))
    {
        string stockName = fileLine.substr(0, fileLine.find(COLON_DELIMITER));
        string price = fileLine.substr(fileLine.find(COLON_DELIMITER) + COLON_DELIMITER.length());
        mktVD0.addStockPrice(stockName, stod(price));
    }

    while (getline(stockPriceDay2, fileLine))
    {
        string stockName = fileLine.substr(0, fileLine.find(COLON_DELIMITER));
        string price = fileLine.substr(fileLine.find(COLON_DELIMITER) + COLON_DELIMITER.length());
        mktVD1.addStockPrice(stockName, stod(price));
    }

    // Task 2: Create a portfolio of bond, swap, European option, and American option
    vector<std::shared_ptr<Trade>> myPortfolio;
    auto bFactory = std::make_unique<BondFactory>();
    auto sFactory = std::make_unique<SwapFactory>();
    auto eFactory = std::make_unique<EurOptFactory>();
    auto aFactory = std::make_unique<AmericanOptFactory>();

    string const portfolioDelimiter = ";";
    cout << "-------------Loading portfolio-------------" << endl;
    while (getline(portfolio, fileLine))
    {
        std::vector<std::string> res = split(fileLine, portfolioDelimiter);
        if (res.size() != 9)
        {
            throw std::invalid_argument("Invalid portfolio initialised: " + fileLine);
        }

        if (res[0] == "Id")
        {
            continue;
        }

        string productType = res[1];
        string productName = res[2];
        string startDateString = res[3];

        Date startDate;
        if (startDateString != "null")
        {
            std::vector<std::string> startDateRes = split(startDateString, "-");

            if (startDateRes.size() != 3)
            {
                throw std::invalid_argument("Invalid Start Date initialised: " + startDateString);
            }

            startDate.year = std::stoi(startDateRes[0]);
            startDate.month = std::stoi(startDateRes[1]);
            startDate.day = std::stoi(startDateRes[2]);
        }

        string endDateString = res[4];
        std::vector<std::string> endDateRes = split(endDateString, DATE_DELIMITER);

        if (endDateRes.size() != 3)
        {
            throw std::invalid_argument("Invalid End Date initialised: " + endDateString);
        }

        Date endDate;
        endDate.year = std::stoi(endDateRes[0]);
        endDate.month = std::stoi(endDateRes[1]);
        endDate.day = std::stoi(endDateRes[2]);

        double notional = std::stod(res[5]);
        string frequencyStr = res[6];
        double frequency = 1;
        if (frequencyStr != "null")
        {
            frequency = std::stod(frequencyStr);
        }
        double strike = std::stod(res[7]);
        cout << "strike: " << strike << endl;
        string opt = res[8];
        OptionType optionType;
        if (opt == "null")
        {
            optionType = OptionType::None;
        }
        else if (opt == "call")
        {
            optionType = OptionType::Call;
        }
        else if (opt == "put")
        {
            optionType = OptionType::Put;
        }

        // Create trades based on the product type and add to the portfolio
        if (productType == "bond")
        {
            auto bond = bFactory->createTrade(productName, startDate, endDate, notional, strike, frequency, optionType);
            bond->setName(productName);
            myPortfolio.push_back(bond);
        }
        else if (productType == "swap")
        {
            auto swap_ = sFactory->createTrade(productName, startDate, endDate, notional, strike, frequency, optionType);
            swap_->setName(productName);
            myPortfolio.push_back(swap_);
        }
        else if (productType == "eur-opt")
        {
            auto eCall = eFactory->createTrade(productName, startDate, endDate, notional, strike, frequency, optionType);
            eCall->setName(productName);
            myPortfolio.push_back(eCall);
        }
        else if (productType == "am-opt")
        {
            auto aPut = aFactory->createTrade(productName, startDate, endDate, notional, strike, frequency, optionType);
            aPut->setName(productName);
            myPortfolio.push_back(aPut);
        }
    }
    cout << "-------------Finish loading " << myPortfolio.size() << " portfolio !-------------" << endl;

    // Task 3: Create a pricer and price the portfolio, output the pricing result of each deal
    // 3.1 Compute the NPV of deal as of market date 1
    // 3.2 Compute the NPV of deal as of market date 2, then compute the daily PnL for each deal using NPV(date2) - NPV(date1), and output the result in file

    auto pricer = new CRRBinomialTreePricer(100);
    string const outputFolder = "output/";
    string const outputType = "_pv";

    string const mktVD0FileName = outputFolder + mktVD0.asOf.toString() + outputType + ".txt";
    string const mktVD1FileName = outputFolder + mktVD1.asOf.toString() + outputType + ".txt";
    string const mktNPVFileName = outputFolder + "NPV" + ".txt";

    ofstream mktVD0File(mktVD0FileName);
    unordered_map<string, double> pvDay1;
    unordered_map<string, double> pvFinal;

    cout << "-------------------------- Calculating PV for " << mktVD0.asOf.toString() << " --------------------------" << endl;

    if (mktVD0File.is_open())
    {
        try
        {
            for (size_t i = 0; i < myPortfolio.size(); i++)
            {
                auto &trade = myPortfolio[i];
                double pv = pricer->Price(mktVD0, trade);
                pvDay1[to_string(i)] = pv;
                mktVD0File << fixed << showpoint;
                mktVD0File << pv << endl;
            }
        }
        catch (const exception &e)
        {
            cout << "Exception " << e.what() << endl;
        }

        mktVD0File.close();
    }
    else
        cerr << "Unable to open file " + mktVD0FileName << endl;

    // Calculating PV for the second market date
    cout << "-------------------------- Calculating PV for " << mktVD1.asOf.toString() << " --------------------------" << endl;
    ofstream mktVD1File(mktVD1FileName);
    if (mktVD1File.is_open())
    {
        try
        {
            for (size_t i = 0; i < myPortfolio.size(); i++)
            {
                auto &trade = myPortfolio[i];
                double pv = pricer->Price(mktVD1, trade);
                pvFinal[to_string(i)] = pv - pvDay1[to_string(i)];
                mktVD1File << fixed << showpoint;
                mktVD1File << pv << endl;
            }
        }
        catch (const exception &e)
        {
            cout << "Exception " << e.what() << endl;
        }
        mktVD1File.close();
    }
    else
        cerr << "Unable to open file " + mktVD1FileName << endl;

    // Calculate NPV, compute the daily PnL for each deal using NPV(date2) - NPV(date1), and output the result in file
    cout << "-------------------------- Calculating NPV --------------------------" << endl;
    ofstream mktNPVFile(mktNPVFileName);
    if (mktNPVFile.is_open())
    {
        try
        {
            for (size_t i = 0; i < pvFinal.size(); i++)
            {
                mktNPVFile << fixed << showpoint;
                mktNPVFile << pvFinal[to_string(i)] << endl;
            }
        }
        catch (const exception &e)
        {
            cout << "Exception " << e.what() << endl;
        }
        mktNPVFile.close();
    }
    else
        cerr << "Unable to open file " + mktNPVFileName << endl;

    // Task 4: Compute the Greeks of DV01 and Vega risk as of market date 1
    // 4.1 Compute risk using risk engine
    // 4.2 Use idea of multi-threading

    cout << "-------------------------- Computing Greeks of DV01 and Vega risk as of " << mktVD0.asOf.toString() << " --------------------------" << endl;

    double curve_shock_up = 0.0001;    // 1 bp of zero rate
    double curve_shock_down = -0.0001; // 1 bp of zero rate
    double vol_shock_up = 0.01;        // 1% of log normal vol
    double vol_shock_down = -0.01;     // 1% of log normal vol
    double price_shock_up = 1.0;       // shock in abs price of stock
    double price_shock_down = 1.0;     // shock in abs price of stock

    for (size_t i = 0; i < myPortfolio.size(); i++)
    {
        auto &trade = myPortfolio[i];
        RiskEngine re(mktVD0, curve_shock_up, vol_shock_up, price_shock_up);
        re.computeRisk("dv01", trade, true);
        auto dv01_of_trade = re.getResult();
        // Further processing for dv01_of_trade can be added here if needed
    }


    cout << "Project build successfully!" << endl;

    return 0;
}
