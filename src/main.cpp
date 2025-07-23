#include <cstring>
#include <memory>
#include <optional>
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

	auto interface = std::make_unique<HashtInterface<int, int>>(argv[1], 30);

	auto input = new InputHandler<int, int>(std::move(interface), my_string_to_int, my_string_to_int, std::to_string);

	input->run();
}

