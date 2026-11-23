#pragma once

#include <string>
#include <vector>
#include <crawlerdefines.h>

class Conranshop_ukParser
{
public:
    void parseItemsList(const std::string& page, UrlList& links);
    void parseNextPage(const std::string& page, std::string& nextUrl);
    void parseItem(const std::string& page, std::map<std::string, std::string>& secResponses, ItemDetails& item);
    void parseItemPrices(const std::string& page, ItemPrices& prices);
private:
};

