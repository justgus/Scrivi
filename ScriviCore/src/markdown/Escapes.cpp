#include "markdown/Escapes.hpp"

namespace scrivi::markdown::escapes {

namespace {

constexpr char16_t kBackslash = u'\\';
constexpr char16_t kNewline = u'\n';

// True when the backslash at `i` is HIDDEN when presented: it escapes one of the 32, or it is a CommonMark HARD LINE BREAK
// (a backslash before a newline that a non-blank line follows).
bool hidesBackslash(UnitsView u, std::size_t i, bool continues) {
    if (u[i] != kBackslash || i + 1 >= u.size()) { return false; }
    if (isEscapable(u[i + 1])) { return true; }
    if (u[i + 1] != kNewline) { return false; }
    std::size_t j = i + 2;
    if (j == u.size()) { return continues; }
    while (j < u.size() && (u[j] == u' ' || u[j] == u'\t')) { ++j; }
    return j < u.size() && u[j] != kNewline;
}

} // namespace

bool isEscapable(char16_t c) {
    constexpr std::u16string_view kMarks = u"!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
    return c < 0x80 && kMarks.find(c) != std::u16string_view::npos;
}

Map map(UnitsView source, bool continues) {
    const std::size_t n = source.size();
    Map m;
    m.presentedToSource.reserve(n + 1);
    m.sourceToPresented.assign(n + 1, 0);
    m.presented.reserve(n);
    std::size_t i = 0;
    while (i < n) {
        const int p = static_cast<int>(m.presentedToSource.size());
        if (hidesBackslash(source, i, continues)) {
            m.presentedToSource.push_back(static_cast<int>(i));   // the caret sits before the backslash
            m.sourceToPresented[i] = p;
            m.sourceToPresented[i + 1] = p;                       // between `\` and its mark → before the mark
            m.presented.push_back(source[i + 1]);
            i += 2;
        } else {
            m.presentedToSource.push_back(static_cast<int>(i));
            m.sourceToPresented[i] = p;
            m.presented.push_back(source[i]);
            i += 1;
        }
    }
    m.presentedToSource.push_back(static_cast<int>(n));
    m.sourceToPresented[n] = static_cast<int>(m.presentedToSource.size()) - 1;
    return m;
}

} // namespace scrivi::markdown::escapes
