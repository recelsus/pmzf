#pragma once

#include "pmzf/pubmed/client.hpp"
#include "spagyrist/spagyrist.hpp"

#include <vector>

namespace pmzf::pubmed {

[[nodiscard]] std::vector<spagyrist::candidate>
to_candidates(const std::vector<SummaryItem>& items);

[[nodiscard]] spagyrist::document
to_document(const SummaryItem& item);

} // namespace pmzf::pubmed
