/**
 *  OSM
 *  Copyright (C) 2026  Pavel Smokotnin
 */
#include "spllogger.h"

#include <QDateTime>
#include <QDir>
#include <QLocale>
#include <QTextStream>
#include <QDebug>

#include "model/metertablemodel.h"

SplLogger::SplLogger(QObject *parent) : QObject(parent), m_file(filePath())
{
    m_timer.setInterval(LoggingIntervalMs);
    connect(&m_timer, &QTimer::timeout, this, &SplLogger::writeSample);
}

bool SplLogger::running() const
{
    return m_timer.isActive();
}

QString SplLogger::filePath() const
{
    return QDir::root().filePath(QStringLiteral("OpenSoundMeter-spl.csv"));
}

QString SplLogger::lastError() const
{
    return m_lastError;
}

void SplLogger::setModel(MeterTableModel *model)
{
    if (m_model == model) {
        return;
    }
    if (running()) {
        stop();
    }
    m_model = model;
}

void SplLogger::start()
{
    if (running()) {
        return;
    }
    if (!m_model) {
        setError(QStringLiteral("The SPL meter model is not available."));
        return;
    }

    m_file.setFileName(filePath());
    if (!m_file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        setError(QStringLiteral("Could not open %1: %2").arg(filePath(), m_file.errorString()));
        return;
    }

    QTextStream stream(&m_file);
    QStringList headers {QStringLiteral("timestamp")};
    for (auto *meter : m_model->meterPlots()) {
        headers.append(csvHeader(meter->sourceName() + QStringLiteral(" ") + meter->title()));
    }
    stream << headers.join(QLatin1Char(',')) << Qt::endl;
    m_file.flush();
    m_lastError.clear();
    emit lastErrorChanged();
    m_timer.start();
    emit runningChanged();
    writeSample();
}

void SplLogger::stop()
{
    if (!running()) {
        return;
    }
    m_timer.stop();
    m_file.close();
    emit runningChanged();
}

void SplLogger::writeSample()
{
    if (!m_model || !m_file.isOpen()) {
        stop();
        return;
    }

    QTextStream stream(&m_file);
    QStringList values {QDateTime::currentDateTime().toString(Qt::ISODate)};
    for (auto *meter : m_model->meterPlots()) {
        bool ok = false;
        const QString displayedValue = meter->value();
        QString numericValue = displayedValue;
        if (numericValue.endsWith(QLatin1Char('%')) || numericValue.endsWith(QChar(0x2030))) {
            numericValue.chop(1);
        }
        const double value = QLocale::c().toDouble(numericValue, &ok);
        values.append(ok ? QLocale::c().toString(value, 'f', 3) : QString());
    }
    stream << values.join(QLatin1Char(',')) << Qt::endl;
    if (stream.status() != QTextStream::Ok) {
        setError(QStringLiteral("Could not write to %1.").arg(filePath()));
        stop();
    }
}

QString SplLogger::csvValue(const QString &value) const
{
    QString escaped = value;
    escaped.replace(QLatin1Char('"'), QStringLiteral("\"\""));
    return QStringLiteral("\"") + escaped + QStringLiteral("\"");
}

QString SplLogger::csvHeader(const QString &value) const
{
    return csvValue(value);
}

void SplLogger::setError(const QString &error)
{
    if (m_lastError == error) {
        return;
    }
    m_lastError = error;
    qWarning() << m_lastError;
    emit lastErrorChanged();
}
