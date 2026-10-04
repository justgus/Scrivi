#include "ManuscriptEscapes.hpp"

// EP-049 — a port of Apple's `ManuscriptEscapes.swift`. Keep the two in step (see the header).

namespace ManuscriptEscapes {

namespace {

const char16_t kBackslash = u'\\';
const char16_t kNewline = u'\n';
const char16_t kSpace = u' ';
const char16_t kTab = u'\t';

// True when the backslash at `i` is HIDDEN when presented: it escapes one of the 32, or it is a
// CommonMark HARD LINE BREAK (a backslash before a newline that a non-blank line follows).
bool hidesBackslash(const QString& u, int i, bool continues)
{
    if (u.at(i).unicode() != kBackslash || i + 1 >= u.size()) {
        return false;
    }
    if (isEscapable(u.at(i + 1))) {
        return true;
    }
    if (u.at(i + 1).unicode() != kNewline) {
        return false;
    }
    int j = i + 2;
    if (j == u.size()) {
        return continues;
    }
    while (j < u.size() && (u.at(j).unicode() == kSpace || u.at(j).unicode() == kTab)) {
        ++j;
    }
    return j < u.size() && u.at(j).unicode() != kNewline;
}

}   // namespace

bool isEscapable(QChar c)
{
    static const QString kMarks = QStringLiteral("!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~");
    return c.unicode() < 0x80 && kMarks.contains(c);
}

QString escape(const QString& typed)
{
    QString out;
    out.reserve(typed.size() * 2);
    for (const QChar c : typed) {
        if (isEscapable(c)) {
            out.append(QChar(kBackslash));
        }
        out.append(c);
    }
    return out;
}

Map map(const QString& source, bool continues)
{
    const int n = source.size();
    Map m;
    m.presentedToSource.reserve(n + 1);
    m.sourceToPresented.fill(0, n + 1);
    m.presented.reserve(n);
    int i = 0;
    while (i < n) {
        const int p = m.presentedToSource.size();
        if (hidesBackslash(source, i, continues)) {
            m.presentedToSource.append(i);   // the caret sits before the backslash
            m.sourceToPresented[i] = p;
            m.sourceToPresented[i + 1] = p;  // between `\` and its mark → before the mark
            m.presented.append(source.at(i + 1));
            i += 2;
        } else {
            m.presentedToSource.append(i);
            m.sourceToPresented[i] = p;
            m.presented.append(source.at(i));
            i += 1;
        }
    }
    m.presentedToSource.append(n);
    m.sourceToPresented[n] = m.presentedToSource.size() - 1;
    return m;
}

QVector<int> hiddenBackslashes(const QString& source, bool continues)
{
    const QVector<int> p2s = map(source, continues).presentedToSource;
    QVector<int> out;
    for (int p = 0; p + 1 < p2s.size(); ++p) {
        if (p2s.at(p + 1) - p2s.at(p) == 2) {
            out.append(p2s.at(p));
        }
    }
    return out;
}

bool isBlankLine(const QString& line)
{
    for (const QChar c : line) {
        const char16_t u = c.unicode();
        if (u != kSpace && u != kTab && u != kNewline) {
            return false;
        }
    }
    return true;
}

}   // namespace ManuscriptEscapes
