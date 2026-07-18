#include "pmzf/pubmed/mapper.hpp"

#include <sstream>

namespace pmzf::pubmed {
namespace {

std::string join_authors(const std::vector<std::string>& authors)
{
    std::ostringstream output;
    for (std::size_t i = 0; i < authors.size(); ++i) {
        if (i > 0) {
            output << ", ";
        }
        output << authors[i];
    }
    return output.str();
}

void add_optional_metadata(
    spagyrist::metadata_map& metadata,
    const std::string& key,
    const std::optional<std::string>& value)
{
    if (value && !value->empty()) {
        metadata.emplace(key, *value);
    }
}

void add_optional_paragraph(
    spagyrist::document& document,
    const std::string& label,
    const std::optional<std::string>& value)
{
    if (value && !value->empty()) {
        document.blocks.push_back(spagyrist::block::paragraph({
            spagyrist::inline_element::strong({spagyrist::inline_element::text_node(label + ": ")}),
            spagyrist::inline_element::text_node(*value),
        }));
    }
}

} // namespace

std::vector<spagyrist::candidate>
to_candidates(const std::vector<SummaryItem>& items)
{
    std::vector<spagyrist::candidate> candidates;
    candidates.reserve(items.size());

    for (const auto& item : items) {
        spagyrist::candidate candidate;
        candidate.id = item.pmid;
        candidate.title = item.title.value_or(item.pmid);
        candidate.url = item.url;
        candidate.metadata.emplace("source", "pubmed");
        candidate.metadata.emplace("pmid", item.pmid);
        add_optional_metadata(candidate.metadata, "journal", item.journal);
        add_optional_metadata(candidate.metadata, "pubdate", item.pubdate);
        add_optional_metadata(candidate.metadata, "doi", item.doi);

        if (item.journal || item.pubdate) {
            std::string subtitle;
            if (item.journal) {
                subtitle += *item.journal;
            }
            if (item.pubdate) {
                if (!subtitle.empty()) {
                    subtitle += " ";
                }
                subtitle += '(' + *item.pubdate + ')';
            }
            candidate.subtitle = subtitle;
        }
        if (!item.authors.empty()) {
            candidate.description = join_authors(item.authors);
            candidate.metadata.emplace("authors", *candidate.description);
        }

        candidates.push_back(std::move(candidate));
    }

    return candidates;
}

spagyrist::document
to_document(const SummaryItem& item)
{
    spagyrist::document document;
    document.metadata.title = item.title.value_or(item.pmid);
    document.metadata.source = "PubMed";
    document.metadata.url = item.url;
    document.metadata.authors = item.authors;
    document.metadata.created_at = item.pubdate;
    document.metadata.extra.emplace("pmid", item.pmid);
    add_optional_metadata(document.metadata.extra, "journal", item.journal);
    add_optional_metadata(document.metadata.extra, "doi", item.doi);

    document.blocks.push_back(spagyrist::block::heading(
        1,
        {spagyrist::inline_element::text_node(document.metadata.title)}));
    document.blocks.push_back(spagyrist::block::paragraph({
        spagyrist::inline_element::strong({spagyrist::inline_element::text_node("PMID: ")}),
        spagyrist::inline_element::link(item.pmid, item.url),
    }));
    document.blocks.push_back(spagyrist::block::paragraph({
        spagyrist::inline_element::strong({spagyrist::inline_element::text_node("URL: ")}),
        spagyrist::inline_element::link(item.url, item.url),
    }));
    add_optional_paragraph(document, "Journal", item.journal);
    add_optional_paragraph(document, "Publication date", item.pubdate);
    add_optional_paragraph(document, "DOI", item.doi);
    if (!item.authors.empty()) {
        document.blocks.push_back(spagyrist::block::paragraph({
            spagyrist::inline_element::strong({spagyrist::inline_element::text_node("Authors: ")}),
            spagyrist::inline_element::text_node(join_authors(item.authors)),
        }));
    }
    if (item.abstract_text && !item.abstract_text->empty()) {
        document.blocks.push_back(spagyrist::block::heading(
            2,
            {spagyrist::inline_element::text_node("Abstract")}));
        document.blocks.push_back(spagyrist::block::paragraph({
            spagyrist::inline_element::text_node(*item.abstract_text),
        }));
    }

    return document;
}

} // namespace pmzf::pubmed
