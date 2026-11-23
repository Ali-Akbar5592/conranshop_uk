#pragma once

#include <webspiders/webspiderbase.h>

class Conranshop_ukSpider : public WebSpiderBase
{
public:
    Conranshop_ukSpider(DBFacade& db, 
                    const CrawlerConfig& config, 
                    boost::asio::io_service& ioService,
                    unsigned maxRequests,
                    const std::string& host, 
                    const std::string& url);

protected:
    virtual CrawlerSearch* buildSearch(DBFacade& db, const CrawlerConfig& config, const SearchParams& params);
};

