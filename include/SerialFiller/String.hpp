///
/// \file 				String.hpp
/// \author 			Geoffrey Hunter (www.mbedded.ninja) <gbmhunter@gmail.com>
/// \edited             n/a
/// \created			2017-08-11
/// \last-modified		2017-09-28
/// \brief 				Contains the String class.
/// \details
///		See README.md in root dir for more info.

#ifndef MN_CPP_UTILS_STRING_H_
#define MN_CPP_UTILS_STRING_H_

// System includes
#include <cstdint>
#include <iomanip>
#include <sstream>

namespace mn {
namespace CppUtils {

/// \brief      Contains static methods for converting various objects to strings.
class String
{
public:
    //etl::to_string(int8_t(127), str, Format().base(16).width(2).fill(STR('0'))));
    template<typename T>
    static std::string ToHex(T iterable)
    {
        // Special formatting when there are no values
        if (iterable.begin() == iterable.end())
            return "{}";

        std::stringstream stream;
        stream << "{ ";

        auto it   = iterable.begin();
        auto end  = iterable.end();
        auto last = end--;
        for (it; it != end; ++it) {
            stream << "0x" << std::setfill('0') << std::setw(sizeof(typename T::value_type) * 2)
                   << std::uppercase << std::hex << (int) (*it);
            //if(it != iterable.end() - 1)
            if (it != last)
                stream << ", ";
        }
        stream << " }";
        return stream.str();
    }
};
} // namespace CppUtils
} // namespace mn

#endif // #ifndef MN_CPP_UTILS_STRING_H_
