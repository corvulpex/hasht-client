#include <cstring>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include "input.h"
#include "interface.h"


std::optional<int> my_string_to_int(std::string s) {
	int i;
	try {
		i = std::stoi(s);
	}
	catch (std::invalid_argument const &ex) {
		return {};
	}
	return std::optional<int>(i);
}

void print_help() {
	std::cout << "You're on your own for now\n";
}


int main(int argc, char* argv[]) {
	for (int i = 1; i < argc; i ++) {
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			print_help();
			return 0;
		}
	}
	if (argc < 2) {
		std::cerr << "No name for shared memory specified\n";
		return 1;
	}

	long long int ports = 31;
	if (argc >= 3) {
		std::string pstr(argv[2]);
		std::stringstream pstream(pstr);
		pstream >> ports;
		if (ports <= 0) {
			std::cerr << "Number of queue ports was not a valid positive number -> will be set to default (30)\n";
			ports = 31;
		}
		else {
			ports++;
		}
	}

	if (argc > 3)
		std::cout << "Extra arguments ignored\n";

	auto interface = std::make_unique<HashtInterface<int, int>>(argv[1], (size_t) ports);

	auto input = new InputHandler<int, int>(std::move(interface), my_string_to_int, my_string_to_int, std::to_string);

	input->run();
}

