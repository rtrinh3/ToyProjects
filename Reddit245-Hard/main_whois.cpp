// https://www.reddit.com/r/dailyprogrammer/comments/3xdmtw/20151218_challenge_245_hard_guess_whois/
#include <iostream>
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

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
// Gonna need something else for other platforms...


namespace Params {
	const char Ranges[] = "ips_sample.txt";
	const char Queries[] = "query_sample.txt";
};

struct IpRange {
	const char* name;
	uint32_t span;
};

struct IpRangeSpanCompare {
	bool operator()(const IpRange& a, const IpRange& b) const {
		return std::tie(a.span, a.name) < std::tie(b.span, b.name);
	}
};

struct RangeBoundMarker {
	IpRange range;
	bool starting;
};

std::unordered_set<std::string> StringPool;
std::vector<uint64_t> Partitions; // Wider than 32bits, to avoid corner cases
std::vector<const char*> PartitionNames;

uint32_t ParseIp(const std::string& ip) {
	auto pass1 = inet_addr(ip.c_str());
	auto pass2 = ntohl(pass1);
	return pass2;
}

void ParseDb() {
	using namespace std::chrono_literals;
	auto startParse = std::chrono::high_resolution_clock::now();
	std::map<uint64_t, std::vector<RangeBoundMarker> > Bounds;
	Bounds[0].push_back({ IpRange{ "<unknown>", 0xffffffff }, true });;

	std::ifstream indb(Params::Ranges);
	std::string left_str;
	std::string right_str;
	std::string name_str;
	while (indb >> left_str >> right_str) {
		indb.ignore();
		std::getline(indb, name_str);
		uint32_t left = ParseIp(left_str);
		uint32_t right = ParseIp(right_str);
		auto interned = StringPool.insert(name_str);

		IpRange newRange{ interned.first->c_str(), (right - left) };
		Bounds[left].push_back({ newRange, true });
		Bounds[right + uint64_t(1)].push_back({ newRange, false });
	}
	auto startPartition = std::chrono::high_resolution_clock::now();
	std::set<IpRange, IpRangeSpanCompare> ActiveRanges;
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
		Partitions.push_back(kvp.first);
		PartitionNames.push_back((ActiveRanges.begin())->name);
	}
	auto end = std::chrono::high_resolution_clock::now();
	std::cout << "Parse:" << ((startPartition - startParse) / 1ms) << "ms Partition:" << ((end - startPartition) / 1ms) << "ms\n";
}

void Stats() {
	std::unordered_map<const char*, size_t> counts;
	std::ifstream reqs(Params::Queries);
	std::string ip_str;
	while (reqs >> ip_str) {
		uint32_t ip = ParseIp(ip_str);

		// Find partition
		auto part = std::upper_bound(Partitions.begin(), Partitions.end(), ip);
		--part; // upper_bound gives us the iterator > our value; we want the iterator <= our value, which would be the previous one.
		auto name = PartitionNames[std::distance(Partitions.begin(), part)];
		counts[name]++;
	}
	std::set<std::pair<size_t, const char*>, std::greater<> > sorted_stats;
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
