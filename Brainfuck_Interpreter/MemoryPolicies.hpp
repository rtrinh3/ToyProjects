#pragma once
#include <deque>
#include <list>
#include <vector>
#include <unordered_map>

template <typename Cell, size_t hint = 1>
class DequeMemoryPolicy {
private:
	std::deque<Cell> Memory = std::deque<Cell>(hint);
	size_t MemoryPointer = 0;
protected:
	Cell& CurrentCell() {
		return Memory[MemoryPointer];
	}
public:
	void Right() {
		++MemoryPointer;
		if (Memory.size() <= MemoryPointer) {
			Memory.push_back(0);
		}
	}
	void Left() {
		if (0 == MemoryPointer) {
			Memory.push_front(0);
		} else {
			--MemoryPointer;
		}
	}
};

template <typename Cell, size_t hint = 1>
class VectorMemoryPolicy {
private:
	std::vector<Cell> Memory = std::vector<Cell>(hint);
	size_t MemoryPointer = 0;
protected:
	Cell& CurrentCell() {
		return Memory[MemoryPointer];
	}
public:
	void Right() {
		++MemoryPointer;
		if (Memory.size() <= MemoryPointer) {
			Memory.push_back(0);
		}
	}
	void Left() {
		if (0 == MemoryPointer) {
			Memory.insert(Memory.cbegin(), 0); // Rarely used but oh so ugly
		} else {
			--MemoryPointer;
		}
	}
};

template <typename Cell, size_t hint = 1>
class ListMemoryPolicy {
private:
	std::list<Cell> Memory = std::list<Cell>(hint);
	typename std::list<Cell>::iterator MemoryPointer = Memory.begin();
protected:
	Cell& CurrentCell() {
		return *MemoryPointer;
	}
public:
	void Right() {
		++MemoryPointer;
		if (Memory.end() == MemoryPointer) {
			Memory.push_back(0);
			--MemoryPointer;
		}
	}
	void Left() {
		if (Memory.begin() == MemoryPointer) {
			Memory.push_front(0);
		}
		--MemoryPointer;
	}
};

template <typename Cell, size_t N = 0x7FFF>
class SafeArrayMemoryPolicy {
private:
	Cell Memory[N] = { 0 };
	Cell* MemoryPointer = Memory;
protected:
	Cell& CurrentCell() {
		return *MemoryPointer;
	}
public:
	void Right() {
		++MemoryPointer;
		if (Memory + N <= MemoryPointer) {
			throw std::runtime_error("Can't move the pointer right");
		}
	}
	void Left() {
		if (Memory == MemoryPointer) {
			throw std::runtime_error("Can't move the pointer left");
		}
		--MemoryPointer;
	}
};

template <typename Cell, size_t N = 0x7FFF>
class UnsafeArrayMemoryPolicy {
private:
	Cell Memory[N] = { 0 };
	Cell* MemoryPointer = Memory;
protected:
	Cell& CurrentCell() {
		return *MemoryPointer;
	}
public:
	void Right() {
		++MemoryPointer;
	}
	void Left() {
		--MemoryPointer;
	}
};

template <typename Cell, size_t Dummy = 0>
class MapMemoryPolicy {
private:
	std::unordered_map<int, Cell> Memory;
	int MemoryPointer = 0;
protected:
	Cell& CurrentCell() {
		return Memory[MemoryPointer];
	}
public:
	void Right() {
		++MemoryPointer;
	}
	void Left() {
		--MemoryPointer;
	}
};
