#include "common.h"

#include "FileManagerClient.h"

int main(int argc, char** argv) {

	try {
		FileManagerClient client("localhost");
		if (argc < 4) {
			throw "Not enough argument, expected: dfs <command> <localName> <dfsName>";
		}

		if (!strcmp(argv[1], "put")) {
			client.putFileCommand(argv[2], argv[3]);
		} else if (!strcmp(argv[1], "get")) {
			client.getFileCommand(argv[2], argv[3]);
		} else {
			throw string("Unknown command: ") + argv[1];
		}

	} catch (const char* exception) {
		cout << "FATAL!" << exception << endl;
	}

	return 0;
}
