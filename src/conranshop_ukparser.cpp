#include <conranshop_uk/include/conranshop_ukparser.h>
#include <parsers/parsercommons.h>
//#include <itemtags.h>
#include <jsonutils.h>
#include <boost/regex.hpp>

using namespace boost;
using namespace boost::property_tree;

void Conranshop_ukParser::parseItemsList(const std::string& page, UrlList& links) 
{
    ItemPrices prices;
    parseItemPrices(page, prices);
    for(auto& pair : prices)
    {
        links.push_back(pair.first);
    }
}

void Conranshop_ukParser::parseNextPage(const std::string& page, std::string& nextUrl) 
{
    static regex NEXT_PAGE("rel=\"next\" href=\"([^<>]*?)\"\\s*/>");
    nextUrl = parseSub(page, NEXT_PAGE, 1);
    if(nextUrl.empty())
    {
        static regex NEXT_PAGE1("<a class=\"next i-next\" href=\"([^<>]*?)\" title=\"Next\">");
        nextUrl = parseSub(page, NEXT_PAGE1, 1);
        htmlDecode(nextUrl);
    }
}

void Conranshop_ukParser::parseItem(const std::string& page, std::map<std::string, std::string>& secResponses, ItemDetails& item) 
{
    static regex ITEM_NAME("class=\"name\">([^<>]*?)</span>");
    item.name = parseSub(page, ITEM_NAME);
    trim(item.name);

    static regex BRAND_REGEX("property=\"og:brand\" content=\"([^<>]*?)\"\\s*/>");
    item.brand = parseSub(page, BRAND_REGEX);
    trim(item.brand);
    if(item.brand.empty())
    {
        static regex BRAND_REGEX1("class=\"product-header-brand product-header-link\">\\s*<a href=\"[^<>\"]*?\">([^<>]*?)<");
        item.brand = parseSub(page, BRAND_REGEX1);
    }

    //static regex PRODUCT_ID_REGEX("class=\"product_id\">([^<>]*?)</span>");
    static regex PRODUCT_SKU_REGEX("Product\\.SKU\\s*=\\s*\"([^<>]*?)\";");
    item.productId = parseSub(page, PRODUCT_SKU_REGEX);

    static regex PRODUCT_PARENT_SKU_REGEX("class=\"product_id\">\\s*([^<>]*?)\\s*</span>");
    item.tags["parent_sku"] = parseSub(page, PRODUCT_PARENT_SKU_REGEX);

    static regex ORIGINAL_PRICE("class=\"price\">\\s*([\\d \\. \\,]+)\\s*</span>");
    item.price = getItemPrice(page, ORIGINAL_PRICE);
    item.tags["regular_price"] = std::to_string(item.price);

    static regex ITEM_BASEPRICE("class=\"list_price\">\\s*([\\d \\. \\,]+)\\s*</span>");
    double baseprice = getItemPrice(page, ITEM_BASEPRICE);
    if (item.price > 0 && baseprice > 0)
    {
        item.discount = baseprice - item.price;
        item.tags["regular_price"] = std::to_string(baseprice);
    }

    if( page.find("Choose Fabric") != std::string::npos)
    {
        item.tags["customizable_tag"] = "yes";
    }
    else
    {
        item.tags["customizable_tag"] = "no";
    }

    //product description
    static regex INFO_REGEX("class=\"description\">(.*?)<span class=");
    item.info = parseSub(page, INFO_REGEX);

    //item features
    static regex FEATURES_REGEX("class=\"product-features\">(.*?)</ul>\\s*</li>\\s*</div>");
    std::string features = parseSub(page, FEATURES_REGEX);
    if(features.empty())
    {
        static regex FEATURES_REGEX1("class=\"product-features\">(.*?)</ul>\\s*</div>\\s*</div>");
        features = parseSub(page, FEATURES_REGEX1);
        removeHtmlTags(features);
        trim(features);
    }
    item.info += features;

    static regex DIMENSION_DIV_REGEX("class=\"product-dimensions-wrapper\"(.*?)</div>\\s*</section>");
    std::string dimensionDiv = parseSub(page, DIMENSION_DIV_REGEX);
    static regex DIMENSION_REGEX("div class=\"original-content\">(.*?)</div>");
    item.tags["dimensions"] = parseSub(dimensionDiv, DIMENSION_REGEX);
    removePartialHtmlTags(item.tags["dimensions"]);
    removeHtmlTags(item.tags["dimensions"]);
    htmlDecode(item.tags["dimensions"]);
    erase_all(item.tags["dimensions"], ">");
    trim(item.tags["dimensions"]);

    //Adding dimension info in item info..
    removeHtmlTags(dimensionDiv);
    trim(dimensionDiv);
    item.info +=  dimensionDiv;

    removePartialHtmlTags(item.info);
    removeHtmlTags(item.info);
    htmlDecode(item.info);
    erase_all(item.info, ">");
    trim(item.info);

    //For specification tags-->
    static regex FABRIC_INFO_REGEX("Fabric Information([^<>]*?)\"\\s*/>");
    std::string fabricInfo = parseSub(page, FABRIC_INFO_REGEX);
    htmlDecode(fabricInfo);
    erase_all(fabricInfo, "Leather Information");
    if(!fabricInfo.empty())
    {
        Strings tagNames;
        Strings tagValues;
        static regex SPECIFICATION_TAGS_REGEX("^(.*?):(.*?)$");
        parseAll(fabricInfo, SPECIFICATION_TAGS_REGEX, 1, tagNames);
        parseAll(fabricInfo, SPECIFICATION_TAGS_REGEX, 2, tagValues);
        for(size_t i = 0; i < tagNames.size(); i++)
        {
            std::string tagName = tagNames[i];
            boost::algorithm::to_lower(tagName);
            boost::algorithm::trim(tagName);

            std::string tagValue = tagValues[i];
            boost::algorithm::to_lower(tagValue);
            boost::algorithm::trim(tagValue);

            item.tags[tagName] = tagValues[i];
            //std::cout<<tagName<<"========="<<item.tags[tagName]<<"\n";
        }

    }

    if(page.find("property=\"og:availability\" content=\"Out of Stock\"") != std::string::npos)
    {
        item.isOut = true;
    }

    static regex IMAGE_URL_REGEX("class=\"image_url\">([^<>]*?)</span>");
    item.imageUrl = parseSub(page, IMAGE_URL_REGEX);

    Strings images;
    parseAll(page, regex("<span class=\"alternate_image_url\">([^<>]*?)</span>"), 1, images);
    item.tags["images"] = join(images, " | ");

    PriceVariant variant;
    //Normal Colour Variant Layout->Unit Tests->conranshop_uk_parse_normal_colour_variant_item
    //,conranshop_uk_parse_normal_colour_variant_item2
    static regex COLOR_JSON_REGEX("new Product\\.Config\\((.*?)\\);\\s*</script>");
    std::string jsonColourDiv = parseSub(page, COLOR_JSON_REGEX);
    htmlDecode(jsonColourDiv);

    if(!jsonColourDiv.empty())
    {
        ptree pt;
        std::stringstream str(jsonColourDiv);
        json_parser::read_json(str, pt);
        std::string attributeId;
        for (auto &v1 : get_child_safe(pt, "attributes"))
        {
            attributeId = v1.first;
        }
        for (auto &v1 : get_child_safe(pt, "attributes." + attributeId + ".options"))
        {
            std::string labelName = json<std::string>(v1.second, "label");
            if(attributeId == "92")
            {
                variant.color = labelName;
            }
            if(attributeId == "289")
            {
                variant.size = labelName;
            }

            float basicPrice = safeLexicalCast<float>(json<std::string>(pt, "basePrice"));
            variant.price = (basicPrice + safeLexicalCast<float>(json<std::string>(v1.second, "price")))*(((json<float>(pt, "taxConfig.currentTax"))/100)+1);
            variant.tags["regular_price"] = std::to_string(variant.price);

            for (auto &v2 : get_child_safe(v1.second, "products"))
            {
                std::string productIdentifier = v2.second.data();
                static regex PRODUCT_DATA_JSON_REGEX("var optionsData\\s*=(.*?);\\s*var");
                std::string dataJson = parseSub(page, PRODUCT_DATA_JSON_REGEX);
                htmlDecode(dataJson);
                removeHtmlTags(dataJson);
                ptree pt1;
                std::stringstream str1(dataJson);
                json_parser::read_json(str1, pt1);
                variant.name = json<std::string>(pt1, productIdentifier + ".name");
                
                item.info = json<std::string>(pt1, productIdentifier + ".descFull") + json<std::string>(pt1, productIdentifier + ".features");
                trim(item.info);

                variant.tags["dimensions"] = json<std::string>(pt1, productIdentifier + ".dimensions");

                item.info = item.info + ". Dimensions: " + variant.tags["dimensions"];
                
                variant.tags["sku"] = json<std::string>(pt1, productIdentifier + ".sku");

                static regex PRODUCT_IMAGES_JSON_REGEX("parseJSON\\('(.*?)'\\)\\);");
                std::string imagesJson = parseSub(page, PRODUCT_IMAGES_JSON_REGEX);
                htmlDecode(imagesJson);
                removeHtmlTags(imagesJson);
                ptree pt2;
                std::stringstream str2(imagesJson);
                json_parser::read_json(str2, pt2);
                variant.image = json<std::string>(pt2, "base_image." + productIdentifier);

                Strings variantImages;
                for (auto &v3 : get_child_safe(pt2, "media_gallery." + productIdentifier))
                {
                    variantImages.push_back(v3.second.data());
                }
                if (!variantImages.empty())
                {
                    variant.tags["images"] = join(variantImages, " | ");
                }
                if((variant.color.size() > 0 || variant.size.size() > 0) && item.isOut == false)
                {
                    item.variants.push_back(variant);
                }
            }
        }
    }

    //Combo Products Style Variants Layout
    static regex ASSOCIATED_PRODUCT_DIV_REGEX("section id=\"associated-products\"(.*?)class=\"associated-products-action associated-products-action--bottom\"");
    std::string comboProductDiv = parseSub(page, ASSOCIATED_PRODUCT_DIV_REGEX);
    htmlDecode(comboProductDiv);

    Strings comboProductList;
    static regex COMBO_PRODUCT_LIST_REGEX("class=\"associated-product-group clearfix\"(.*?)</ul>\\s*</div>\\s*</div>");
    parseAll(comboProductDiv, COMBO_PRODUCT_LIST_REGEX, 1, comboProductList);
    for(auto& productList : comboProductList)
    {
        static regex IMAGE_URL_REGEX("<img src=\"([^<>\"]*?)\" alt=\"[^<>\"]*?\">");
        variant.image = parseSub(productList, IMAGE_URL_REGEX);

        Strings comboProducts;
        static regex COMBO_PRODUCTS_REGEX("class=\"associated-product-item\"(.*?)</div>\\s*</li>");
        parseAll(productList, COMBO_PRODUCTS_REGEX, 1, comboProducts);
        for(auto& product : comboProducts)
        {
            static regex VARIANT_STYLE_REGEX("class=\"associated-product-item-cell associated-product-item-name\">([^<>]*?)<");
            variant.style = parseSub(product, VARIANT_STYLE_REGEX);

            variant.name = item.name + "-" + variant.style;

            static regex VARIANT_DIMENSION_REGEX("class=\"associated-product-item-cell associated-product-item-dimensions\">([^<>]*?)</div>");
            variant.tags["dimensions"] = parseSub(product, VARIANT_DIMENSION_REGEX);
            removeHtmlTags(variant.tags["dimensions"]);
            trim(variant.tags["dimensions"]);

            static regex VARIANT_PRODUCT_SKU_REGEX("class=\"associated-product-item-cell associated-product-item-sku\">\\s*Product Code:([^<>]*?)</div>");
            variant.tags["sku"] = parseSub(product, VARIANT_PRODUCT_SKU_REGEX);
            removeHtmlTags(variant.tags["sku"]);
            trim(variant.tags["sku"]);

            //Combo Variant Price
            static regex VARIANT_PRICE_DIV("class=\"price-box\"(.*?)</div>\\s*</div>");
            std::string variantPriceDiv = parseSub(product, VARIANT_PRICE_DIV);
            
            static regex VARIANT_PRICE_REGEX("<span class=\"regular-price\" id=\"product-price[^<>\"]*?\">\\s*<span class=\"price\">\\s*([\\d \\. \\,]+)\\s*</span>");
            variant.price = getItemPrice(variantPriceDiv, VARIANT_PRICE_REGEX);
            variant.tags["regular_price"] = std::to_string(variant.price);
            if(variant.price == 0)
            {
                static regex VARIANT_PRICE_REGEX2("<span class=\"price\" id=\"product-price[^<>\"]*?\">\\s*([\\d \\. \\,]+)\\s*</span>");
                variant.price = getItemPrice(variantPriceDiv, VARIANT_PRICE_REGEX2);
            }

            static regex VARIANT_BASE_PRICE_REGEX("<span class=\"price\" id=\"old-price[^<>\"]*?\">\\s*([\\d \\. \\,]+)\\s*</span>");
            float variant_basePrice = getItemPrice(variantPriceDiv, VARIANT_BASE_PRICE_REGEX);
            if( variant.price > 0 && variant_basePrice > 0 )
            {
                variant.discount = variant_basePrice - variant.price;
                variant.tags["regular_price"] = std::to_string(variant_basePrice);
            }

            static regex VARIANT_STOCK_LEVEL_REGEX("class=\"stock-message\">([^<>]*?)</small>");
            variant.tags["stock_level"] = parseSub(product, VARIANT_STOCK_LEVEL_REGEX);

            bool availabilityFlag = false;
            if(product.find("class=\"in-stock\"") != std::string::npos)
            {
                availabilityFlag = true;
            }

            if(variant.style.size() > 0 && availabilityFlag)
            {
                item.variants.push_back(variant);
            }
        }
    }

    //Custom Variants Product
    static regex CUSTOM_PRODUCT_JSON_REGEX("new Product\\.Options\\((.*?)\\);\\s*var");
    std::string customProductJson = parseSub(page, CUSTOM_PRODUCT_JSON_REGEX);
    htmlDecode(customProductJson);
    if(!customProductJson.empty())
    {
        static regex PRICE_JSON_REGEX("new Product\\.OptionsPrice\\((.*?)\\);\\s*var");
        std::string priceJson = parseSub(page, PRICE_JSON_REGEX);
        htmlDecode(priceJson);
        ptree ptpr;
        std::stringstream strpr(priceJson);
        json_parser::read_json(strpr, ptpr);
        float basicPrice = json<float>(ptpr, "productPrice");
        float currentTax = json<float>(ptpr, "currentTax");

        ptree pt;
        std::stringstream str(customProductJson);
        json_parser::read_json(str, pt);
        for(auto &v1 : pt)
        {
            std::string swatchId = v1.first;
            for (auto &v1 : get_child_safe(pt, swatchId))
            {
                std::string productIdentity = v1.first;
                float incrementPrice = json<float>(pt, swatchId + "." + productIdentity + ".price");
                variant.price = (basicPrice + incrementPrice)*((currentTax/100) + 1);
                variant.tags["regular_price"] = std::to_string(variant.price);

                variant.tags["sku"] = json<std::string>(pt, swatchId + "." + productIdentity + ".sku");

                variant.color = json<std::string>(pt, swatchId + "." + productIdentity + ".title");
                erase_all(variant.color, ":");
                std::string title = variant.color;
                to_lower(title);
                replace_all(title, " ", "_");

                if(!item.productId.empty() && !title.empty())
                {
                    std::string secUrl = "https://www.conranshop.co.uk/pbuilder/images/getfileslist?sku=" + item.productId + "&title=" + title;
                    item.secondaryUrls.push_back(secUrl);  
                }

                for (auto &pair : secResponses)
                {
                    if (pair.first.find(title) != std::string::npos)
                    {
                        if(isJson(pair.second))
                        {
                            ptree ptimage;
                            std::stringstream strimage(pair.second);
                            json_parser::read_json(strimage, ptimage);
                            for (auto &v1 : get_child_safe(ptimage, ""))
                            {
                                std::string imageUrlPart = v1.second.data();
                                std::string imageUrlFull = "https://iop.conranshop.co.uk/media/wysiwyg/byo/products/" + item.productId + "/options/fabric/" + title + "/" + imageUrlPart;
                                variant.image = imageUrlFull;
                                break;
                            }
                            Strings variantImages;
                            for (auto &v1 : get_child_safe(ptimage, ""))
                            {
                                std::string imageUrlPart = v1.second.data();
                                std::string imageUrlFull = "https://iop.conranshop.co.uk/media/wysiwyg/byo/products/" + item.productId + "/options/fabric/" + title + "/" + imageUrlPart;
                                variantImages.push_back(imageUrlFull);
                            }
                            if (!variantImages.empty())
                            {
                                variant.tags["images"] = join(variantImages, " | ");
                            }
                        }
                        if(variant.image.empty() &&  variant.tags["images"].empty())
                        {
                            variant.tags["images"] = item.tags["images"];
                        }
                        
                        if(variant.tags["images"].empty())
                        {
                            variant.tags["images"] = item.imageUrl;
                        }
                        
                        if(variant.color.size() > 0 && !variant.tags["images"].empty() && item.isOut == false)
                        {
                            item.variants.push_back(variant);
                        }              
                    }
                }
            }
        }
    }
      
    std::vector<std::string> groups;
    static regex GROUPS_DIV("class=\"breadcrumbs\"(.*?)</ul>\\s*</div>");
    static regex GROUPS_NAME(">([^<>]*?)</a>");
    std::string groupsDiv = parseSub(page, GROUPS_DIV, 1);
    parseAll(groupsDiv, GROUPS_NAME, 1, groups);
    for(auto &group : groups)
    {
        removeHtmlTags(group);
        htmlDecode(group);
        trim(group);
    }
    
    item.groups = boost::algorithm::join(groups, " > ");
    if(item.tags["images"].empty())
    {
        item.tags["images"] = item.imageUrl;
    }

    //shipping fees
    item.tags["shipping_fees"] = parseSub(page, regex("class=\"shipping-estimated-value\">(.*?)</span>"));
    removeHtmlTags(item.tags["shipping_fees"]);
    trim(item.tags["shipping_fees"]);
    htmlDecode(item.tags["shipping_fees"]);
}

void Conranshop_ukParser::parseItemPrices(const std::string& page, ItemPrices& prices)
{
    std::vector<std::string> items;
    static const regex ITEM_REGEX("class=\"product-name\"(.*?)</span>\\s*</span>\\s*</div>");
    parseAll(page, ITEM_REGEX, 1, items);
    for (const std::string& item : items)
    {
        static const regex ITEMLIST_URL("href=\"([^<>]*?)\"");
        std::string url = parseSub(item, ITEMLIST_URL);
        ItemPrice itemPrice;
        prices.push_back({url, itemPrice});
    }
}

