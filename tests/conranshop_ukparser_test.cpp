#include <boost/foreach.hpp>
#include <conranshop_uk/include/conranshop_ukparser.h>
#include <parsers/parsercommons.h>
#include <testutils.h>
#include <iostream>

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>

Strings secondaryHeaders = {"x-requested-with: XMLHttpRequest"};

BOOST_AUTO_TEST_CASE( conranshop_uk_parse_searchitem_list )
{
    Conranshop_ukParser parser;
    std::string html = readUrl("https://www.conranshop.co.uk/search/go?w=towels");

    UrlList items;
    parser.parseItemsList(html, items);

    BOOST_CHECK_GE(items.size(), 2 );
    for(const std::string& url : items)
    {
        BOOST_CHECK_GE(url.size(), 10);
    }
}

BOOST_AUTO_TEST_CASE( conranshop_uk_parse_item_list )
{  
    Conranshop_ukParser parser;
    std::string html = readUrl("https://www.conranshop.co.uk/kitchen-and-dining.html?limit=150");

    UrlList items, items2;
    parser.parseItemsList(html, items);
    
    BOOST_CHECK_GE(items.size(), 100 );    
    for(const std::string& url : items)
    {
        BOOST_CHECK_GE(url.size(), 10);
    }
    
    std::string nextPage;
    parser.parseNextPage(html, nextPage);
    
    BOOST_CHECK_EQUAL(nextPage, "https://www.conranshop.co.uk/kitchen-and-dining.html?limit=150&p=2#category-products");

    std::string html2 = readUrl("https://www.conranshop.co.uk/kitchen-and-dining.html?limit=150&p=3#category-products");
    parser.parseItemsList(html2, items2);
    BOOST_CHECK_GE(items2.size(), 100 );

    std::string nextPage1;
    parser.parseNextPage(html2, nextPage1);
    
    BOOST_CHECK_EQUAL(nextPage1, "https://www.conranshop.co.uk/kitchen-and-dining.html?limit=150&p=4#category-products");
}
//Normal Colour Variant Tests
BOOST_AUTO_TEST_CASE( conranshop_uk_parse_normal_colour_variant_item )
{
    Conranshop_ukParser parser;
    std::string html = readUrl("https://www.conranshop.co.uk/skittle-bottle.html");
    std::map<std::string, std::string> secondaryResponse;
    ItemDetails item;
    parser.parseItem(html, secondaryResponse, item);
    BOOST_CHECK_EQUAL(item.name, "Skittle Thermos Flask");
    BOOST_CHECK_EQUAL(item.productId, "Skittle_Bottle");
    BOOST_CHECK_EQUAL(item.info.empty(), false);
    BOOST_CHECK_GE(item.price, 10);
    BOOST_CHECK_GE(item.discount, 0);
    BOOST_CHECK_EQUAL(item.imageUrl, "https://iop.conranshop.co.uk/media/catalog/product/1/3/1332155.jpg");
    BOOST_CHECK_EQUAL(item.brand, "Lund London");
    BOOST_CHECK_EQUAL(item.groups, "Home > Kitchen & Dining");
    BOOST_CHECK_GE(item.variants.size(), 1);
    BOOST_CHECK_EQUAL(item.isOut, false);
    PriceVariant variant;
    for(PriceVariant& variant : item.variants)
    {
        BOOST_CHECK_EQUAL(variant.name.empty(), false);
        BOOST_CHECK_EQUAL(variant.tags["dimensions"].empty(), false);
        BOOST_CHECK_EQUAL(variant.tags["sku"].empty(), false);
        BOOST_CHECK_GE(variant.tags["images"].size(), 4);
        BOOST_CHECK_EQUAL(variant.image.empty(), false);
        BOOST_CHECK_EQUAL(variant.color.empty(), false);
        BOOST_CHECK_GE(variant.price, 10);
    }
}

BOOST_AUTO_TEST_CASE( conranshop_uk_parse_normal_colour_variant_item2 )
{
    Conranshop_ukParser parser;
    std::string html = readUrl("https://www.conranshop.co.uk/material-pendant-light.html");
    std::map<std::string, std::string> secondaryResponse;
    ItemDetails item;
    parser.parseItem(html, secondaryResponse, item);
    BOOST_CHECK_EQUAL(item.name, "Material Pendant Light");
    BOOST_CHECK_EQUAL(item.productId, "Material_Pendant_Light");
    BOOST_CHECK_EQUAL(item.info.empty(), false);
    BOOST_CHECK_GE(item.price, 10);
    BOOST_CHECK_GE(item.discount, 0);
    BOOST_CHECK_EQUAL(item.imageUrl, "https://iop.conranshop.co.uk/media/catalog/product/1/3/1311785_1.jpg");
    BOOST_CHECK_EQUAL(item.brand, "Noergaard & Kechayas");
    BOOST_CHECK_EQUAL(item.groups, "Home");
    BOOST_CHECK_GE(item.variants.size(), 3);
    BOOST_CHECK_EQUAL(item.isOut, false);
    PriceVariant variant;
    for(PriceVariant& variant : item.variants)
    {
        BOOST_CHECK_EQUAL(variant.name.empty(), false);
        BOOST_CHECK_EQUAL(variant.tags["sku"].empty(), false);
        BOOST_CHECK_EQUAL(variant.tags["dimensions"].empty(), false);
        BOOST_CHECK_GE(variant.tags["images"].size(), 4);
        BOOST_CHECK_EQUAL(variant.image.empty(), false);
        BOOST_CHECK_EQUAL(variant.color.empty(), false);
        BOOST_CHECK_GE(variant.price, 10);
    }
}

//Combo product variant tests
BOOST_AUTO_TEST_CASE( conranshop_uk_parse_combo_colour_variant_item )
{
    Conranshop_ukParser parser;
    std::string html = readUrl("https://www.conranshop.co.uk/bathroom-linen-collection-tapenade.html");
    std::map<std::string, std::string> secondaryResponse;
    ItemDetails item;
    parser.parseItem(html, secondaryResponse, item);
    BOOST_CHECK_EQUAL(item.name, "Premium Terry Towel Collection Tapenade");
    BOOST_CHECK_EQUAL(item.productId, "The_Conran_Shop_Bathroom_Linen_Collection_Tapenade");
    BOOST_CHECK_EQUAL(item.info.empty(), false);
    BOOST_CHECK_GE(item.price, 3);
    BOOST_CHECK_GE(item.discount, 0);
    BOOST_CHECK_EQUAL(item.imageUrl, "https://iop.conranshop.co.uk/media/catalog/product/t/h/the_conran_shop_bathroom_linen_collection_tapenade.jpg");
    BOOST_CHECK_EQUAL(item.brand, "The Conran Shop");
    BOOST_CHECK_EQUAL(item.groups, "Home");
    BOOST_CHECK_GE(item.variants.size(), 3);
    BOOST_CHECK_EQUAL(item.isOut, false);
    PriceVariant variant;
    for(PriceVariant& variant : item.variants)
    {
        BOOST_CHECK_EQUAL(variant.name.empty(), false);
        BOOST_CHECK_EQUAL(variant.tags["sku"].empty(), false);
        BOOST_CHECK_EQUAL(variant.tags["dimensions"].empty(), false);
        BOOST_CHECK_EQUAL(variant.image.empty(), false);
        BOOST_CHECK_GE(variant.price, 3);
    }
}

BOOST_AUTO_TEST_CASE( conranshop_uk_parse_combo_colour_variant_item2 )
{
    Conranshop_ukParser parser;
    std::string html = readUrl("https://www.conranshop.co.uk/conran-4k-grouped.html");
    std::map<std::string, std::string> secondaryResponse;
    ItemDetails item;
    parser.parseItem(html, secondaryResponse, item);
    BOOST_CHECK_EQUAL(item.name, "4K Mattress");
    BOOST_CHECK_EQUAL(item.productId, "conran_4k_grouped");
    BOOST_CHECK_EQUAL(item.info.empty(), false);
    BOOST_CHECK_GE(item.price, 10);
    BOOST_CHECK_GE(item.discount, 0);
    BOOST_CHECK_EQUAL(item.imageUrl, "https://iop.conranshop.co.uk/media/catalog/product/c/o/conran_4k_0_3.jpeg");
    BOOST_CHECK_EQUAL(item.brand, "The Conran Shop");
    BOOST_CHECK_EQUAL(item.groups, "Home");
    BOOST_CHECK_GE(item.variants.size(), 1);
    BOOST_CHECK_EQUAL(item.isOut, false);
    PriceVariant variant;
    for(PriceVariant& variant : item.variants)
    {
        BOOST_CHECK_EQUAL(variant.name.empty(), false);
        BOOST_CHECK_EQUAL(variant.tags["sku"].empty(), false);
        BOOST_CHECK_EQUAL(variant.tags["dimensions"].empty(), false);
        BOOST_CHECK_EQUAL(variant.image.empty(), false);
        BOOST_CHECK_GE(variant.price, 5);
    }
}

BOOST_AUTO_TEST_CASE( conranshop_uk_parse_combo_colour_variant_item3 )
{
    Conranshop_ukParser parser;
    std::string html = readUrl("https://www.conranshop.co.uk/bathroom-linen-collection-tapenade.html");
    std::map<std::string, std::string> secondaryResponse;
    ItemDetails item;
    parser.parseItem(html, secondaryResponse, item);
    BOOST_CHECK_EQUAL(item.name, "Premium Terry Towel Collection Tapenade");
    BOOST_CHECK_EQUAL(item.productId, "The_Conran_Shop_Bathroom_Linen_Collection_Tapenade");
    BOOST_CHECK_EQUAL(item.info.empty(), false);
    BOOST_CHECK_GE(item.price, 3);
    BOOST_CHECK_GE(item.discount, 0);
    BOOST_CHECK_EQUAL(item.imageUrl, "https://iop.conranshop.co.uk/media/catalog/product/t/h/the_conran_shop_bathroom_linen_collection_tapenade.jpg");
    BOOST_CHECK_EQUAL(item.brand, "The Conran Shop");
    BOOST_CHECK_EQUAL(item.groups, "Home");
    BOOST_CHECK_GE(item.variants.size(), 2);
    BOOST_CHECK_EQUAL(item.isOut, false);
    PriceVariant variant;
    for(PriceVariant& variant : item.variants)
    {
        BOOST_CHECK_EQUAL(variant.name.empty(), false);
        BOOST_CHECK_EQUAL(variant.tags["sku"].empty(), false);
        BOOST_CHECK_EQUAL(variant.tags["dimensions"].empty(), false);
        BOOST_CHECK_EQUAL(variant.image.empty(), false);
        BOOST_CHECK_GE(variant.price, 3);
        BOOST_CHECK_GE(variant.discount, 1);
    }
}

//Custom product variants
/*Custom product variants unique case url where secondary urls for images are reduntant-->
https://www.conranshop.co.uk/caravaggio-p4-pendant-light-6m-cord-spec-colour.html*/
BOOST_AUTO_TEST_CASE( conranshop_uk_parse_custom_colour_variant_item )
{
    Conranshop_ukParser parser;
    std::string html = readUrl("https://www.conranshop.co.uk/clarence-bed-150cm.html");
    std::map<std::string, std::string> secondaryResponse;

    std::string secondaryUrl1 = "https://www.conranshop.co.uk/pbuilder/images/getfileslist?sku=clarence_bed_150cm&title=linen_chalk";
    std::string secondaryUrl2 = "https://www.conranshop.co.uk/pbuilder/images/getfileslist?sku=clarence_bed_150cm&title=cotton_whitewash";
    std::string secondaryUrl3 = "https://www.conranshop.co.uk/pbuilder/images/getfileslist?sku=clarence_bed_150cm&title=velvet_verdigris";
    std::string secondaryUrl4 = "https://www.conranshop.co.uk/pbuilder/images/getfileslist?sku=clarence_bed_150cm&title=wool_navy";
    std::string secondaryUrl5 = "https://www.conranshop.co.uk/pbuilder/images/getfileslist?sku=clarence_bed_150cm&title=duro_velvet_nickel";

    std::string secondaryResponse1 = readUrl(secondaryUrl1, secondaryHeaders);
    std::string secondaryResponse2 = readUrl(secondaryUrl2, secondaryHeaders);
    std::string secondaryResponse3 = readUrl(secondaryUrl3, secondaryHeaders);
    std::string secondaryResponse4 = readUrl(secondaryUrl4, secondaryHeaders);
    std::string secondaryResponse5 = readUrl(secondaryUrl5, secondaryHeaders);

    secondaryResponse[secondaryUrl1] = secondaryResponse1;
    secondaryResponse[secondaryUrl2] = secondaryResponse2;
    secondaryResponse[secondaryUrl3] = secondaryResponse3;
    secondaryResponse[secondaryUrl4] = secondaryResponse4;
    secondaryResponse[secondaryUrl5] = secondaryResponse5;

    ItemDetails item;
    parser.parseItem(html, secondaryResponse, item);
    
    BOOST_CHECK_EQUAL(item.name, "Clarence Bed Queen Size");
    BOOST_CHECK_EQUAL(item.productId, "clarence_bed_150cm");
    BOOST_CHECK_EQUAL(item.info.empty(), false);
    BOOST_CHECK_GE(item.price, 5);
    BOOST_CHECK_GE(item.discount, 0);
    BOOST_CHECK_EQUAL(item.imageUrl, "https://iop.conranshop.co.uk/media/catalog/product/1/_/1_1308983_clarence_150cm_bed_duro_velvet_amber_front.jpg");
    BOOST_CHECK_EQUAL(item.brand, "The Conran Shop");
    BOOST_CHECK_EQUAL(item.groups, "Home");
    BOOST_CHECK_GE(item.variants.size(), 3);
    BOOST_CHECK_EQUAL(item.isOut, false);
    PriceVariant variant;
    for(PriceVariant& variant : item.variants)
    {
        BOOST_CHECK_EQUAL(variant.color.empty(), false);
        BOOST_CHECK_EQUAL(variant.tags["sku"].empty(), false);
        BOOST_CHECK_EQUAL(variant.tags["images"].empty(), false);
        BOOST_CHECK_EQUAL(variant.image.empty(), false);
        BOOST_CHECK_GE(variant.price, 5);
    }
}

BOOST_AUTO_TEST_CASE( conranshop_uk_parse_item )
{
    Conranshop_ukParser parser;
    std::string html = readUrl("https://www.conranshop.co.uk/cross-plex-table-lamp-t-300.html");
    std::map<std::string, std::string> secondaryResponse;
    ItemDetails item;
    parser.parseItem(html, secondaryResponse, item);
    BOOST_CHECK_EQUAL(item.name, "T-300 Cross-Plex Table Lamp");
    BOOST_CHECK_EQUAL(item.productId, "1319088");
    BOOST_CHECK_EQUAL(item.info.empty(), false);
    BOOST_CHECK_GE(item.price, 10);
    BOOST_CHECK_GE(item.discount, 0);
    BOOST_CHECK_EQUAL(item.imageUrl, "https://iop.conranshop.co.uk/media/catalog/product/1/3/1319088.jpg");
    BOOST_CHECK_EQUAL(item.brand, "Bodil Kjær");
    BOOST_CHECK_EQUAL(item.groups, "Home");
    BOOST_CHECK_EQUAL(item.isOut, false);
    BOOST_CHECK_EQUAL(item.tags["images"].empty(), false);
    BOOST_CHECK_EQUAL(item.tags["dimensions"].empty(), false);
}
