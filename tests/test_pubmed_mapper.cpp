#include "pmzf/pubmed/mapper.hpp"

#include "test_support.hpp"

namespace {

pmzf::pubmed::SummaryItem make_summary_item()
{
    pmzf::pubmed::SummaryItem item;
    item.pmid = "12345";
    item.title = "Sample article";
    item.journal = "Example Journal";
    item.pubdate = "2026";
    item.doi = "10.0000/example";
    item.authors = {"A Author", "B Author"};
    item.url = "https://pubmed.ncbi.nlm.nih.gov/12345/";
    return item;
}

void maps_summary_to_candidate()
{
    const auto candidates = pmzf::pubmed::to_candidates({make_summary_item()});

    PMZF_CHECK(candidates.size() == 1);
    PMZF_CHECK(candidates.front().id == "12345");
    PMZF_CHECK(candidates.front().title == "Sample article");
    PMZF_CHECK(candidates.front().url == "https://pubmed.ncbi.nlm.nih.gov/12345/");
    PMZF_CHECK(candidates.front().metadata.at("source") == "pubmed");
    PMZF_CHECK(candidates.front().metadata.at("pmid") == "12345");
}

void maps_summary_to_document()
{
    const auto document = pmzf::pubmed::to_document(make_summary_item());

    PMZF_CHECK(document.metadata.title == "Sample article");
    PMZF_CHECK(document.metadata.source == "PubMed");
    PMZF_CHECK(document.metadata.url == "https://pubmed.ncbi.nlm.nih.gov/12345/");
    PMZF_CHECK(document.metadata.authors.size() == 2);
    PMZF_CHECK(document.metadata.extra.at("pmid") == "12345");
    PMZF_CHECK(!document.blocks.empty());
}

} // namespace

void run_pubmed_mapper_tests()
{
    maps_summary_to_candidate();
    maps_summary_to_document();
}
