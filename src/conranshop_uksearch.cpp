#include <builders/httpbuilder.h>
#include <clog.h>
#include <conranshop_uk/include/conranshop_ukparser.h>
#include <conranshop_uk/include/conranshop_uksearch.h>
#include <dbfacade/dbfacade.h>
#include <performance/Performance.h>

#include <parsers/parsercommons.h>

Conranshop_ukSearch::Conranshop_ukSearch(DBFacade& db, const CrawlerConfig& config, const SearchParams& params)
    : CrawlerSearch(db, config, params)
{
    
}

void Conranshop_ukSearch::deserializeResponse(const std::string& response, const HttpParams&) 
{
    PROFILE_FUNC();

    Conranshop_ukParser parser;
    
    if (_urlClass == "item")
    {
        ItemDetails item;
        parser.parseItem(response, _secondaryResponses, item);
        addItem(item);
        return;
    }
    
    if (_urlClass == "item_lists")
    {
        UrlList links;
        parser.parseItemsList(response, links);
        if(_url.find("search/go?w=") != std::string::npos)
        {
            stableUniqueVector(links);
            if(links.size() > 50)
            {
                links.resize(50);
            }
        }
        addLinks(links, "item");
        LOG_DEBUG(SEARCH, "Items found: " << links.size() << " url: " << _url);

        if (links.empty())
        {
            ItemDetails item;
            parser.parseItem(response, _secondaryResponses, item);
            if (item.name.size() > 0)
            {
                addItem(item);
            }
            return;
        }

        if (links.empty())
            return;

        if(_url.find("search/go?w=") == std::string::npos)
        {
            std::string nextPage;
            parser.parseNextPage(response, nextPage);
            addLink(nextPage, "item_lists");
        }
    }
}

void Conranshop_ukSearch::addSecondaryHeaders(const std::string &, std::vector<std::string> &requestHeaders)
{
    requestHeaders.push_back("x-requested-with: XMLHttpRequest");
}

bool Conranshop_ukSearch::validItemUrl(const std::string& url) const
{
    //return url.find("/us/p/") != std::string::npos;
    return true;
}
bool Conranshop_ukSearch::validUrl(const std::string& url) const
{
	//return url.find("/us/p/") != std::string::npos;
	return true;
}
