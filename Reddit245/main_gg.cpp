//https://www.reddit.com/r/dailyprogrammer/comments/3x3hqa/20151216_challenge_245_intermediate_ggggggg_gggg/

#include <iostream>
#include <memory>
#include <unordered_map>
#include <string>
class PrefixTree {
private:
	using ValueType = char;
	struct Node {
		~Node() = default;
		Node() = default;
		std::unordered_map<char, std::unique_ptr<Node> > children;
		ValueType value = 0;
	};
	Node root;
public:
	PrefixTree() = default;
	void Insert(const std::string& str, ValueType v) {
		Node* node = &root;
		size_t i = 0;
		const size_t n = str.length();
		// Lookup
		while (i < n) {
			const auto it = node->children.find(str[i]);
			if (it != node->children.end()) {
				node = it->second.get();
				++i;
			} else {
				break;
			}
		}
		// New nodes
		while (i < n) {
			auto insertResult = node->children.insert(std::make_pair(str[i], std::make_unique<Node>()));
			node = insertResult.first->second.get();
			++i;
		}
		if (node->value) {
			std::cerr << "Warning: String " << str << " : Replacing value " << node->value << " with " << v << std::endl;
		}
		node->value = v;
	}

	// Given a string, finds the longest prefix, returns the iterator to the 
	// position after that prefix and the corresponding value.
	template <class Iterator>
	std::pair<Iterator, ValueType>
		FindLongest(Iterator str, Iterator last) const
	{
		const Node* node = &root;
		while (str != last) {
			const auto it = node->children.find(*str);
			if (it == node->children.end()) {
				return std::make_pair(str, (node ? node->value : 0));
			} else {
				node = it->second.get();
			}
			++str;
		}
		return std::make_pair(str, (node ? node->value : 0));
	}
};

struct HuffmanNode {
	char symbol = 0;
	size_t weight = 0;
	HuffmanNode* parent = nullptr;
	std::shared_ptr<HuffmanNode> left = nullptr;
	std::shared_ptr<HuffmanNode> right = nullptr;
};
using PHuffmanNode = std::shared_ptr<HuffmanNode>;
struct PHuffmanNodeComp {
	bool operator()(const PHuffmanNode& l, const PHuffmanNode& r) const {
		return l->weight > r->weight;
	}
};



#include <string>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <ciso646>
#include <queue>
#include <map>
#include <cctype>
int main() {
	/*
	PrefixTree trie;
	std::string keyLine;
	std::getline(std::cin, keyLine);
	std::stringstream keyLineStream(keyLine);
	while (keyLineStream.good()) {
		char c;
		std::string str;
		keyLineStream >> c >> str;
		trie.Insert(str, c);
	}

	std::string msg;
	using Itr = std::istreambuf_iterator<char>;
	std::copy(Itr(std::cin), Itr{}, std::back_inserter(msg));
	auto pos = msg.begin();
	auto last = msg.end();
	while (pos != last) {
		auto p = trie.FindLongest(pos, last);
		if (p.first == pos) {
			std::cout << *pos;
			++pos;
		} else {
			std::cout << p.second;
			pos = p.first;
		}
	}
	std::cout << "<\n";
	*/

	// Encoder
	// Stringify message
	std::string msg;
	using Itr = std::istreambuf_iterator<char>;
	std::copy(Itr(std::cin), Itr{}, std::back_inserter(msg));
	// Frquencies
	std::map<char, size_t> freq;
	for (unsigned char c : msg) {
		if (std::isalpha(c)) {
			freq[c]++;
		}
	}
	// Huffman tree
	std::priority_queue<PHuffmanNode, std::vector<PHuffmanNode>, PHuffmanNodeComp> theQueue;
	std::map<char, PHuffmanNode> charToNode;
	for (auto&& kvp : freq) {
		auto newNode = std::make_shared<HuffmanNode>();
		newNode->symbol = kvp.first;
		newNode->weight = kvp.second;
		theQueue.push(newNode);
		charToNode.insert(decltype(charToNode)::value_type(kvp.first, newNode));
	}
	while (theQueue.size() > 1) {
		auto a(theQueue.top());
		theQueue.pop();
		auto b(theQueue.top());
		theQueue.pop();

		auto newNode = std::make_shared<HuffmanNode>();
		newNode->left = a;
		a->parent = newNode.get();
		newNode->right = b;
		b->parent = newNode.get();
		newNode->weight = a->weight + b->weight;

		theQueue.push(newNode);
	}
	// Huffman codes
	std::map<char, std::string> HuffmanCodes;
	for (auto&& kvp : charToNode) {
		char c = kvp.first;
		const HuffmanNode* node = kvp.second.get();
		while (node->parent != nullptr) {
			HuffmanCodes[c].insert(0, 1, (
				(node == node->parent->left.get()) 
				? 'g' 
				: 'G'
				));
			node = node->parent;
		}
	}
	
	// Encoding
	for (auto&& kvp : HuffmanCodes) {
		std::cout << kvp.first << ' ' << kvp.second << ' ';
	}
	std::cout << std::endl;
	for (char c : msg) {
		auto it = HuffmanCodes.find(c);
		if (it != HuffmanCodes.end()) {
			std::cout << it->second;
		} else {
			std::cout << c;
		}
	}
}
