#include "StdAfx.hpp"
#include "FeedValidator.hpp"
#include "MemParseHandlers.hpp"

#include <fstream>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xalanc/XalanTransformer/XalanTransformer.hpp>

namespace RssReader
{
	FeedValidator::FeedValidator(const std::string& feedData, Feed& feed) :
		_feedData(feedData),
		_feed(feed),
		_validationValid(true)
	{
	}

	std::string FeedValidator::Validate()
	{
		std::string transformed = ValidateFeedFormat("RSS_20_xsl.txt", Rss20);
		if (!transformed.empty())
			return transformed;

		transformed = ValidateFeedFormat("Atom_10_xsl.txt", Atom10);
		if (!transformed.empty())
			return transformed;

		transformed = ValidateFeedFormat("Atom_03_xsl.txt", Atom03);
		if (!transformed.empty())
			return transformed;

		transformed = ValidateFeedFormat("RDF_xsl.txt", RDF);
		if (!transformed.empty())
			return transformed;

		_feed.Valid = false;
		_feed.Format = Unknown;
		return std::string();
	}

	std::string FeedValidator::ValidateFeedFormat(const std::string& xslPath, FeedFormat feedFormat)
	{
		std::ifstream xslFile(xslPath.c_str(), std::ios_base::binary | std::ios_base::in);
		std::stringstream xslStream;
		xslFile.get(*xslStream.rdbuf(), '\0');
		xslStream.seekg(std::stringstream::beg);

		const std::string transformed = TransformFeed(xslStream);
		if (!transformed.empty() && ValidateFeed(transformed))
		{
			_feed.Format = feedFormat;
			_feed.Valid = true;
			return transformed;
		}
		
		return std::string();
	}

	std::string FeedValidator::TransformFeed(std::stringstream& xslStream)
	{
		std::stringstream result;
		xalanc_1_10::XalanTransformer trans;
		std::stringstream xmlStream(_feedData);
		const xalanc_1_10::XSLTInputSource xmlIn(xmlStream);
		const xalanc_1_10::XSLTInputSource xslIn(xslStream);
		const xalanc_1_10::XSLTResultTarget xmlOut(result);
		trans.transform(xmlIn, xslIn, xmlOut);
		return result.str();
	}

	bool FeedValidator::ValidateFeed(const std::string& feed) const
	{
		MemParseHandlers handler;
		const xercesc_2_7::MemBufInputSource input(reinterpret_cast<const XMLByte*>(feed.c_str()), feed.size(), "c:/");

		xercesc_2_7::XercesDOMParser parser;
		//parser.setValidationScheme(xercesc_2_7::XercesDOMParser::Val_Always);
		parser.setValidationScheme(xercesc_2_7::XercesDOMParser::Val_Never);
		//parser.setDoNamespaces(true);
		//parser.setDoSchema(true);
		//parser.setSkipDTDValidation(true);
		//parser.setExternalNoNamespaceSchemaLocation(xercesc_2_7::XMLString::transcode("Feed_XSD.txt"));
		parser.setErrorHandler(&handler);

		parser.parse(input);

		return handler.GetSuccess();
	}
}
