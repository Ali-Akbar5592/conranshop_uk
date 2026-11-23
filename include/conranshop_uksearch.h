#pragma once

#include <searchobjects/crawlersearch.h>

/*!
 * \brief The Conranshop_uk search
 */
class Conranshop_ukSearch: public CrawlerSearch
{
public:
    Conranshop_ukSearch(DBFacade& db, const CrawlerConfig& config, const SearchParams& params);

    /*!
     * \brief Parse response
     */
    virtual void deserializeResponse(const std::string& response, const HttpParams& params);
    void addSecondaryHeaders(const std::string &, std::vector<std::string> &requestHeaders);
    bool validItemUrl(const std::string& url) const override;
    bool validUrl(const std::string& url) const;
};

