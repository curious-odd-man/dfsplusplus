#include "common.h"

#include "FileManagerClient.h"

int main(int argc, char** argv) {

	try {
		FileManagerClient client("localhost");
		client.putFileCommand(argv[1]);
	} catch (const char* exception) {
		cout << "FATAL!" << exception << endl;
	}

	return 0;
}
