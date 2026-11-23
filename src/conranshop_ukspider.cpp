#include <conranshop_uk/include/conranshop_ukspider.h>
#include <conranshop_uk/include/conranshop_uksearch.h>

Conranshop_ukSpider::Conranshop_ukSpider(DBFacade& db, const CrawlerConfig& config, boost::asio::io_service& ioService, unsigned maxRequests, const std::string& host, const std::string& url)
    : WebSpiderBase(db,config,ioService, maxRequests, host, url)
{
}

CrawlerSearch* Conranshop_ukSpider::buildSearch(DBFacade& db, const CrawlerConfig& config, const SearchParams& params) 
{
    return new Conranshop_ukSearch(_db, _config, params);
}
