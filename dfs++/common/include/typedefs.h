#pragma once

#include "common.h"


enum CMD {
	PUT,
	GET,

	UNKNOWN
};

struct CmdPackage {

	CmdPackage();

	int index;			// To track query/response
	CMD command;		// Command to execute
	char fname[255];	// Name of the file
	size_t dataSize;	// size of data

	string toString() const;
};

struct FilePart {

	FilePart();

	long index;
	long replica_id;
	size_t offset;
	size_t size;
	char host[255];
	unsigned short port;

	string toString() const;
};

struct DataChunk {
	long index;
	long replicat_id;
	size_t offset;
	size_t size;
};
