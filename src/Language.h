#include <unordered_map>
#include <string>
#include <vector>

#include "cereal/types/unordered_map.hpp"

struct Lang
{
    std::string                                     LangName;
    std::unordered_map<std::string, std::string>    LangMap;

    template <class Archive>
    void serialize(Archive & ar)
    {
        ar(
            CEREAL_NVP(LangName),
            CEREAL_NVP(LangMap)
        );
    }
};

class LanguageSelect
{
public:
    LanguageSelect();

    const std::string &         getString(uint8_t language, const std::string & label);
    void                        getLanguages(std::vector<std::string> & vect);
    size_t                      size();

    template <class Archive>
    void serialize(Archive & ar)
    {
        ar(
            CEREAL_NVP(LocalLanguage)
        );
    }
private:
    std::vector< struct Lang > LocalLanguage;
};

extern class LanguageSelect GlobalLanguage;
#define GETSTR(l, n) (GlobalLanguage.getString(l, n))