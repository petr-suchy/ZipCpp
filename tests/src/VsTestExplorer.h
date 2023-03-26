#pragma once

#include <string>

#ifdef VS_TEST_EXPLORER

// replaces a suite name with Test Explorer format 'namespace.class.'
static std::string replace_boost_suite_name(
    const std::string& suite_name
)
{
    std::string out;

    // find delimiter between namespace and class
    std::size_t pos = suite_name.find("__");
    std::size_t len = suite_name.length();

    if (pos != std::string::npos && // delimiter found
        pos > 0 && pos + 2 < len // and has character(s) before and after
    ) {
        out = suite_name.substr(0, pos) + // namespace
            '.' + suite_name.substr(pos + 2) + '.'; // .class.
    }
    else {
        out = suite_name;
    }

    return out;
}

#ifdef BOOST_AUTO_TEST_SUITE_WITH_DECOR

// redefine the macro used for automated test suite registration

#undef BOOST_AUTO_TEST_SUITE_WITH_DECOR

#define BOOST_AUTO_TEST_SUITE_WITH_DECOR( suite_name, decorators )      \
namespace suite_name {                                                  \
BOOST_AUTO_TU_REGISTRAR( suite_name )(                                  \
    replace_boost_suite_name(#suite_name).c_str(),                      \
    __FILE__, __LINE__,                                                 \
    decorators );                                                       \
/**/

#endif

#endif
