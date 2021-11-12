#pragma once
#include <map>
#include <list>
#include <vector>

#include "CacheReader.h"
#define MAX_CHUNK_SIZE 1 << 16

class CacheReader;

struct messageChunk {
	char *buf;
	int length;
};

struct cacheEntry {
	char *url;
	std::list<messageChunk> chunks;
	bool isFull;
};

struct listenerEntry {
    char *url;
    CacheReader *listener;
};

class Cache
{
private:
	std::mutex entriesMutex;
	std::list<cacheEntry> entries;

	std::mutex listenersMutex;
	std::vector<listenerEntry> listeners;

public:
	void addEntry(char* url);

	void addChunk(char* url, messageChunk chunk);

	bool contains(char* url);

	std::list<messageChunk> getChunks(char* url);

	bool entryIsFull(char* url);

	void makeEntryFull(char* url);

	void listenToUrl(char* url, CacheReader *listener);

	void stopListening(CacheReader *listener);

	void clear();

	Cache();
	~Cache();
};

