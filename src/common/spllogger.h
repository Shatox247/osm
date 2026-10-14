/**
 *  OSM
 *  Copyright (C) 2026  Pavel Smokotnin
 */
#ifndef SPLLOGGER_H
#define SPLLOGGER_H

#include <QFile>
#include <QObject>
#include <QString>
#include <QTimer>

class MeterTableModel;

class SplLogger : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(QString filePath READ filePath CONSTANT)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    explicit SplLogger(QObject *parent = nullptr);

    bool running() const;
    QString filePath() const;
    QString lastError() const;

    Q_INVOKABLE void setModel(MeterTableModel *model);
    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();

signals:
    void runningChanged();
    void lastErrorChanged();

private slots:
    void writeSample();

private:
    static constexpr int LoggingIntervalMs = 1000;

    QString csvValue(const QString &value) const;
    QString csvHeader(const QString &value) const;
    void setError(const QString &error);

    MeterTableModel *m_model = nullptr;
    QFile m_file;
    QTimer m_timer;
    QString m_lastError;
};

#endif // SPLLOGGER_H
