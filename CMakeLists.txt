cmake_minimum_required(VERSION 3.28)
project(Archive)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(Archive
        AmericanTrade.h
        Bond.cpp
        Bond.h
        ConfigManager.cpp
        ConfigManager.h
        Constants.cpp
        Constants.h
        Date.cpp
        Date.h
        EuropeanTrade.h
        Factory.h
        helper.h
        main.cpp
        Market.cpp
        Market.h
        Payoff.h
        Pricer.cpp
        Pricer.h
        RiskEngine.cpp
        RiskEngine.h
        Swap.cpp
        Swap.h
        thread_pool.h
        Trade.h
        TreeProduct.h
        Types.h)
