#include "context.h"
#include "domain/domain_pipeline.h"
#include "enum/enum_pipline.h"
#include "sql/sql_pipeline.h"
#include "verbose/verbose_pipeline.h"

#include <iostream>
#include <memory>
#include <stdexcept>

int main(int argc, char* argv[]) {
    Context ctx;
    ctx.init(argc, argv);
    if (ctx) {
        ctx.print_usage();
        return 1;
    }

    try {
        switch (ctx.get_call_type()) {
            case CallType::DOMAIN:
                std::cout << "[WARNING]: The command line option '-enum-domain' is deprecated. "
                          << std::endl;
                return 0;
            case CallType::ENUM:
                std::cout << "[WARNING]: The command line option '-enum' is deprecated. "
                          << std::endl;
                return 0;
            case CallType::SQL:
                std::cout << "[WARNING]: The command line option '-sql' is deprecated. "
                          << std::endl;
                return 0;
            case CallType::VERBOSE: {
                auto verbose_flags = std::static_pointer_cast<VerboseFlags const>(ctx.get_flags());
                verbose::process_verbose(*verbose_flags);
                return 0;
            }
            default:
                ctx.print_usage();
                return 1;
        }
    } catch (std::exception const& e) {
        std::cerr << "SwiftCode error: " << e.what() << '\n';
        return 1;
    }
}
