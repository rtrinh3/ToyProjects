// https://www.reddit.com/r/dailyprogrammer/comments/3xdmtw/20151218_challenge_245_hard_guess_whois/
#include <iostream>
#include <regex>
#include <string>
#include <unordered_set>
#include <set>
#include <vector>
#include <fstream>
#include <iterator>
#include <unordered_map>
#include <map>
#include <algorithm>
#include <tuple>
#include <functional>
#include <chrono>
#include <list>

namespace Params {
	const char Ranges[] = "ips_sample.txt";
	const char Queries[] = "query_sample.txt";
};

struct IpRange {
	const char* name;
	uint32_t span;
};

struct IpRangeSpanCompare {
	bool operator()(const IpRange* a, const IpRange* b) const {
		return a->span < b->span;
	}
};

struct RangeBoundMarker {
	const IpRange* range;
	bool starting;
};

std::unordered_set<std::string> StringPool;
std::vector<uint64_t> Partitions; // Wider than 32bits, to avoid corner cases
std::vector<const char*> PartitionNames;

size_t ParseNumber(const std::pair<std::string::const_iterator, std::string::const_iterator>& str) {
	size_t ans = 0;
	for (auto first = str.first; first != str.second; ++first) {
		ans = (ans * 10) + (*first - '0');
	}
	return ans;
}

void ParseDb() {
	std::list<IpRange> Ranges;
	std::map<uint64_t, std::vector<RangeBoundMarker> > Bounds;
	Ranges.push_back(IpRange{ "<unknown>", 0xffffffff });
	Bounds[0].push_back({ &Ranges.back(), true });

	std::ifstream indb(Params::Ranges);
	std::string line;
	std::regex pattern(R"=((\d+).(\d+).(\d+).(\d+) (\d+).(\d+).(\d+).(\d+) (.*))=");
	std::smatch match;
	while (indb.good()) {
		std::getline(indb, line);
		if (std::regex_match(line, match, pattern)) {
			uint32_t left = (ParseNumber(match[1]) << 24) | (ParseNumber(match[2]) << 16) | (ParseNumber(match[3]) << 8) | (ParseNumber(match[4]));
			uint32_t right = (ParseNumber(match[5]) << 24) | (ParseNumber(match[6]) << 16) | (ParseNumber(match[7]) << 8) | (ParseNumber(match[8]));
			std::string name(match[9].first, match[9].second);
			auto interned = StringPool.insert(name);

			Ranges.push_back(IpRange{ interned.first->c_str(), (right - left) });
			Bounds[left].push_back({ &Ranges.back(), true });
			Bounds[right + uint64_t(1)].push_back({ &Ranges.back(), false });
		}
	}

	std::set<const IpRange*, IpRangeSpanCompare> ActiveRanges;
	for (auto&& kvp : Bounds) {
		// Determine currently active ranges
		for (auto&& bound : kvp.second) {
			if (bound.starting) {
				ActiveRanges.insert(bound.range);
			} else {
				ActiveRanges.erase(bound.range);
			}
		}
		// Select the smallest one, and mark it
		const char* name = (*ActiveRanges.begin())->name;
		Partitions.push_back(kvp.first);
		PartitionNames.push_back(name);
	}
}

void Stats() {
	std::unordered_map<const char*, size_t> counts;
	std::ifstream reqs(Params::Queries);
	std::string line;
	std::regex pattern(R"=((\d+).(\d+).(\d+).(\d+))=");
	std::smatch match;
	while (reqs.good()) {
		std::getline(reqs, line);
		if (std::regex_match(line, match, pattern)) {
			uint32_t ip = (ParseNumber(match[1]) << 24) | (ParseNumber(match[2]) << 16) | (ParseNumber(match[3]) << 8) | (ParseNumber(match[4]));

			// Find partition
			auto part = std::upper_bound(Partitions.begin(), Partitions.end(), ip);
			--part; // upper_bound gives us the iterator > our value; we want the iterator <= our value, which would be the previous one.
			auto name = PartitionNames[std::distance(Partitions.begin(), part)];
			counts[name]++;
		}
	}
	std::set<std::pair<size_t, std::string>, std::greater<> > sorted_stats;
	for (auto&& kvp : counts) {
		sorted_stats.insert({ kvp.second, kvp.first });
	}
	for (auto&&kvp : sorted_stats) {
		std::cout << kvp.first << " - " << kvp.second << '\n';
	}
}

int main() {
	using namespace std::chrono_literals;
	auto startDb = std::chrono::high_resolution_clock::now();
	ParseDb();
	auto start = std::chrono::high_resolution_clock::now();
	Stats();
	auto end = std::chrono::high_resolution_clock::now();
	std::cout << "Building ranges: " << ((start - startDb) / 1ms) << "ms\n";
	std::cout << "Queries: " << ((end - start) / 1ms) << "ms\n";
}
