#include "XlsxBuilder.hpp"

#include "TitleHeaders.hpp"

#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QVersionNumber>
#include <QtDebug>

XlsxBuilder::XlsxBuilder(InOutParameter parameter) :
    Builder{ std::move(parameter) }
{
    if (!m_ioParameter.outputFile.endsWith("xlsx")) {
        m_ioParameter.outputFile += ".xlsx";
    }
}

auto XlsxBuilder::build(const Result &res) const -> bool
{
    QXlsx::Document xlsx;
    int row{ 1 };
    int col{ 1 };

    addTsSupport(row, col, xlsx);

    xlsx.write(row-1, 2, res.params.source_lang);
    xlsx.write(row-1, 3, res.params.lang);

    const char *header[] = { TitleHeader::Context, 
                             TitleHeader::Source,
                             TitleHeader::Translation,
                             TitleHeader::TranslationPlural,
                             TitleHeader::TranslationComment,
                             TitleHeader::Location
    };
    for (int i = 0; i < 6; i++)
        xlsx.write(row, i + 1, header[i]);

    col = 1;
    //if (row == 1) {
        ++row;
    //}
    for (const auto &tr : res.translantions) {
        for (const auto &msg : tr.messages) {
            xlsx.write(row, col++, tr.name);
            xlsx.write(row, col++, msg.source);
            xlsx.write(row, col++, msg.translation);
            xlsx.write(row, col++, msg.translation_plural);
            xlsx.write(row, col++, msg.comment);

            for (const auto &loc : msg.locations) {
                xlsx.write(
                    row, col++,
                    QString(loc.first + " - " + QString::number(loc.second)));
            }
            ++row;
            col = 1;
        }
    }

    if (!xlsx.saveAs(m_ioParameter.outputFile)) {
        qWarning() << "error writing file";
        return false;
    }

    return true;
}

void XlsxBuilder::addTsSupport(int &row, int &col, QXlsx::Document &doc) const
{
    const auto appVersion       = qApp->applicationVersion();
    const auto currentVersion   = QVersionNumber::fromString(appVersion);
    const auto TsSupportVersion = QVersionNumber(4, 5, 0);
    if (QVersionNumber::compare(currentVersion, TsSupportVersion) >= 0) {
        doc.write(row, col, TitleHeader::TsVersion);
        ++row;
        doc.write(row, col, m_ioParameter.tsVersion);
        ++row;
    }
}
