#include "TsBuilder.hpp"

#include <QFile>
#include <QXmlStreamWriter>
#include <QtDebug>

TsBuilder::TsBuilder(InOutParameter parameter) : Builder{ std::move(parameter) }
{
    if (!m_ioParameter.outputFile.endsWith("ts")) {
        m_ioParameter.outputFile += ".ts";
    }
}

auto TsBuilder::build(const Result &res) const -> bool
{
    QFile output(m_ioParameter.outputFile);
    if (!output.open(QFile::WriteOnly | QFile::Truncate)) {
        qWarning() << "can't open file" << output.fileName();
        return false;
    }

    QXmlStreamWriter s(&output);
    s.setAutoFormatting(true);
    s.setAutoFormattingIndent(4);
    s.writeStartDocument();

    s.writeEmptyElement("!DOCTYPE TS");
    s.writeStartElement("TS");
    s.writeAttribute("version", res.params.tsVersion);
    if (!res.params.source_lang.isEmpty())
        s.writeAttribute("sourcelanguage", res.params.source_lang);
    if (!res.params.lang.isEmpty())
        s.writeAttribute("language", res.params.lang);
    for (const auto &ctxs : res.translantions) {
        s.writeStartElement("context");
        s.writeTextElement("name", ctxs.name);

        for (const auto &msg : ctxs.messages) {
            s.writeStartElement("message");

            if (!msg.translation_plural.isEmpty())
                s.writeAttribute("numerus", "yes");

            for (const auto &loc : msg.locations) {
                s.writeEmptyElement("location");
                s.writeAttribute("filename", loc.first);
                s.writeAttribute("line", QString::number(loc.second));
            }

            s.writeTextElement("source", msg.source);

            s.writeStartElement("translation");
            if (msg.translation.isEmpty())
                s.writeAttribute("type", "unfinished");
            if (!msg.translation_plural.isEmpty()) {
                s.writeTextElement("numerusform", msg.translation);
                s.writeTextElement("numerusform", msg.translation_plural);
            } else {
                s.writeCharacters(msg.translation);
            }
            s.writeEndElement();

            if (!msg.comment.isEmpty())
                s.writeTextElement("extracomment", msg.comment);
            s.writeEndElement(); // message
        }
        s.writeEndElement(); // context
    }

    s.writeEndDocument();
    output.close();
    removeSlashInDoctype(&output);
    return true;
}

void TsBuilder::removeSlashInDoctype(QFile *f)
{
    f->open(QIODevice::ReadWrite);
    auto fileData = f->readAll();
    QString text(fileData);
    text.replace("!DOCTYPE TS/", "!DOCTYPE TS");
    f->seek(0);
    f->write(text.toUtf8());
    f->close();
}
