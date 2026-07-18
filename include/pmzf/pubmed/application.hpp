#pragma once

#include <iosfwd>
#include <string>
#include <vector>

namespace pmzf::pubmed {

[[nodiscard]] std::string application_usage();

int run(const std::vector<std::string>& args, std::ostream& out, std::ostream& err);

} // namespace pmzf::pubmed
