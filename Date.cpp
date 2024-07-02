#include "Date.h"

// this function return the year franction between 2 date object
double operator-(const Date &d1, const Date &d2)
{
  int yearDiff = d1.year - d2.year;
  int monthDiff = (d1.month - d2.month);
  int dayDiff = d1.day - d2.day;
  return yearDiff + monthDiff / 12.0 + dayDiff / 360.0;
};

bool operator>(const Date &d1, const Date &d2)
{
  double gap = d1 - d2;
  if (gap > 0)
    return true;
  else
    return false;
};

bool operator<(const Date &d1, const Date &d2)
{
  double gap = d1 - d2;
  if (gap < 0)
    return true;
  else
    return false;
};

bool operator>=(const Date &d1, const Date &d2)
{
  return !(d1 < d2);
}

bool operator<=(const Date &d1, const Date &d2)
{
  return !(d1 > d2);
}

bool operator==(const Date &d1, const Date &d2)
{
  double gap = d1 - d2;
  if (gap == 0)
    return true;
  else
    return false;
};
double yearsBetween(const Date &d1, const Date &d2)
{
  double yearsDifference = d2.year - d1.year;

  // Adjust for months
  yearsDifference += (d2.month - d1.month) / 12.0;

  // Adjust for days within the month
  int daysInCurrentMonth = daysInMonthOfYear(d1.year, d1.month);
  yearsDifference += (d2.day - d1.day) / static_cast<double>(daysInCurrentMonth);

  return yearsDifference;
}

int daysInMonthOfYear(int year, int month)
{
  switch (month)
  {
  case 4:
  case 6:
  case 9:
  case 11:
    return 30;
  case 2:
    // Check for leap year
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))
      return 29;
    else
      return 28;
  default:
    return 31;
  }
}

std::ostream &operator<<(std::ostream &os, const Date &d)
{
  os << d.year << " " << d.month << " " << d.day << std::endl;
  return os;
}

std::istream &operator>>(std::istream &is, Date &d)
{
  is >> d.year >> d.month >> d.day;
  return is;
}

string Date::toString() const
{
  return std::to_string(year) + "-" + std::to_string(month) + "-" + std::to_string(day);
}

Date dateAddTenor(const Date &start, const string &tenorStr)
{
  Date newDate = Date(start.year, start.month, start.day);

  auto tenorUnit = tenorStr.back();

  if (toLower(tenorStr) == "on" || toLower(tenorStr) == "o/n")
  {
    newDate.day += 1;
    if (newDate.day > 30)
    {
      newDate.month += 1;
      newDate.day = 1;
    }
  }
  else if (tenorUnit == 'W')
  {
    int numUnit = stoi(tenorStr.substr(0, tenorStr.size() - 1));
    newDate.day += numUnit * 7;
    if (newDate.day > 30)
    {
      newDate.month += 1;
      newDate.day = newDate.day - 30;
    }
  }
  else if (tenorUnit == 'M')
  {
    int numUnit = stoi(tenorStr.substr(0, tenorStr.size() - 1));
    newDate.month += numUnit;
    if (newDate.month > 12)
    {
      newDate.year += 1;
      newDate.month -= 12;
    }
  }
  else if (tenorUnit == 'Y')
  {
    int numUnit = stoi(tenorStr.substr(0, tenorStr.size() - 1));
    newDate.year += numUnit;
  }
  else
  {
    throw std::runtime_error("Error: found unsupported tenor: " + tenorStr);
  }

  return newDate;
}
