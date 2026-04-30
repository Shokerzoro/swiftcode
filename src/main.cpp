
#include <iostream>
#include <stdexcept>

int main(int argc, char* argv[]) {

    // Command line parsing
	// First arg should be -enum / -sql

	// if -enum require one arg with directory path
	try {
		// Calls enum::process_enums()
	}
	catch (std::exception& e) {
		std::cerr << "CodeGen during enum exeption: " << e.what() << std::endl;
	}

	// if -sql require require two args with input and output dir
	try {
		// Calls sql::process_sql()
	}
	catch (std::exception& e) {
		std::cerr << "CodeGen during sql exeption: " << e.what() << std::endl;
	}

    return 0;
}