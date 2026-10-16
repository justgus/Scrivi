#include "ManuscriptTypography.hpp"

#include <QFile>
#include <QFontDatabase>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <algorithm>

namespace BundledFonts {

namespace {

Manifest& mutableManifest()
{
    static Manifest m = [] {
        Manifest out;
        QFile f(QStringLiteral(":/fonts/fonts.json"));
        if (!f.open(QIODevice::ReadOnly)) {
            qWarning("[BundledFonts] fonts.json missing from the resources — the manuscript falls back to the system font");
            return out;
        }
        const QJsonObject root = QJsonDocument::fromJson(f.readAll()).object();
        out.defaultFace = root.value(QStringLiteral("default")).toString();
        for (const QJsonValue& v : root.value(QStringLiteral("faces")).toArray()) {
            const QJsonObject o = v.toObject();
            Face face;
            face.name = o.value(QStringLiteral("name")).toString();
            face.role = o.value(QStringLiteral("role")).toString();
            face.folder = o.value(QStringLiteral("folder")).toString();
            face.variable = o.value(QStringLiteral("variable")).toBool();
            for (const QJsonValue& fv : o.value(QStringLiteral("files")).toArray()) {
                const QJsonObject fo = fv.toObject();
                face.files.append({fo.value(QStringLiteral("file")).toString(), fo.value(QStringLiteral("italic")).toBool(),
                                   fo.value(QStringLiteral("weight")).toInt(400)});
            }
            out.faces.append(face);
        }
        return out;
    }();
    return m;
}

}  // namespace

const Manifest& manifest()
{
    return mutableManifest();
}

void registerAll()
{
    static bool done = false;
    if (done) {
        return;
    }
    done = true;
    int failed = 0;
    for (Face& face : mutableManifest().faces) {
        for (const FontFile& file : face.files) {
            const int id = QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/%1/%2").arg(face.folder, file.file));
            if (id < 0) {
                ++failed;
                continue;
            }
            if (face.family.isEmpty()) {
                const QStringList families = QFontDatabase::applicationFontFamilies(id);
                face.family = families.isEmpty() ? QString() : families.first();
            }
        }
    }
    if (failed > 0) {
        qWarning("[BundledFonts] %d font file(s) not registered", failed);
    }
}

const Face* face(const QString& name)
{
    const auto& faces = manifest().faces;
    auto it = std::find_if(faces.cbegin(), faces.cend(), [&](const Face& f) { return f.name == name; });
    if (it == faces.cend()) {
        it = std::find_if(faces.cbegin(), faces.cend(), [](const Face& f) { return f.name == manifest().defaultFace; });
    }
    return it == faces.cend() ? nullptr : &*it;
}

}  // namespace BundledFonts

ManuscriptTypography::ManuscriptTypography()
    : faceName(BundledFonts::manifest().defaultFace)
{
}

ManuscriptTypography::ManuscriptTypography(QString name, double pointSize)
    : faceName(std::move(name)), size(std::clamp(pointSize, kMinSize, kMaxSize))
{
}

QFont ManuscriptTypography::bodyFont() const
{
    QFont f;
    if (const BundledFonts::Face* drawn = face(); drawn != nullptr && !drawn->family.isEmpty()) {
        f = QFont(drawn->family);
    }
    f.setPointSizeF(size);
    return f;
}
