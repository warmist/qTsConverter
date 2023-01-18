#include "TsParser.hpp"

#include <QFile>
#include <QtXml>

TsParser::TsParser(InOutParameter &&parameter) : Parser{ std::move(parameter) }
{
}
bool is_numerus(const QDomNode& node) {
    auto attrs = node.attributes();
    if (attrs.contains("numerus"))
        return true;
    return false;
}
auto TsParser::parse() const -> Result
{
    QDomDocument doc;
    QFile file(m_ioParameter.inputFile);
    InOutParameter param;
    if (!file.open(QIODevice::ReadOnly) || !doc.setContent(&file)) {
        return Result{ "Failed to open source!", {}, {} };
    }

    Translations translations;
    //parse source target language
    auto TS = doc.elementsByTagName(QStringLiteral("TS")).item(0);
    auto attrs = TS.attributes();
    param.lang = attrs.namedItem("language").nodeValue();
    param.source_lang = attrs.namedItem("sourcelanguage").nodeValue();
    //
    auto contexts = doc.elementsByTagName(QStringLiteral("context"));
    for (int i = 0; i < contexts.size(); ++i) {
        auto nodeCtx = contexts.item(i);
        TranslationContext context;
        context.name = nodeCtx.firstChildElement(QStringLiteral("name")).text();
        auto msgs    = nodeCtx.childNodes();
        for (auto j = 0; j < msgs.size(); j++) {
            auto nodeMsg   = msgs.item(j);
            auto locations = nodeMsg.childNodes();
            if (nodeMsg.nodeName() != QStringLiteral("message")) {
                continue;
            }

            TranslationMessage msg;

            msg.source =
                nodeMsg.firstChildElement(QStringLiteral("source")).text();
            auto translation_node =
                nodeMsg.firstChildElement(QStringLiteral("translation"));
            if (is_numerus(nodeMsg)) {
                auto numerus_nodes = translation_node.childNodes();
                msg.translation        = numerus_nodes.at(0).toElement().text();
                msg.translation_plural = numerus_nodes.at(1).toElement().text();
            }
            else
            {
                msg.translation = translation_node.text();
            }

            auto comment_node =
                nodeMsg.firstChildElement(QStringLiteral("extracomment"));
            if (!comment_node.isNull())
                msg.comment = comment_node.text();


            for (int k = 0; k < locations.size(); k++) {
                if (locations.at(k).nodeName() == "location") {
                    auto locMsg = locations.item(k);
                    msg.locations.emplace_back(wrapLocation(locMsg));
                }
            }
            context.messages.emplace_back(msg);
        }
        translations.emplace_back(context);
    }

    return Result{ "", std::move(translations), std::move(param) };
}

auto TsParser::wrapLocation(const QDomNode &node) -> std::pair<QString, int>
{
    auto location = node.toElement();
    auto fn       = location.attributeNode(QStringLiteral("filename")).value();
    auto line     = location.attributeNode(QStringLiteral("line")).value();
    return std::make_pair(fn, line.toInt());
}
