//
// Created by michaelpollind on 3/13/17.
//
#include "o0keychainstore.h"

#include <QDebug>
#include <keychain.h>
#include <QtCore/QDataStream>
#include <QtCore/QBuffer>
#include <QtCore/QEventLoop>

using namespace QKeychain;

o0keyChainStore::o0keyChainStore(const QString& app,const QString& name,QObject *parent):
    O0AbstractStore(parent), app_(app),name_(name),pairs_()
{
}

QString o0keyChainStore::value(const QString &key, const QString &defaultValue) {
    return pairs_.value(key, defaultValue);
}

void o0keyChainStore::setValue(const QString &key, const QString &value) {
    pairs_.insert(key,value);
}

void o0keyChainStore::persist() {
    WritePasswordJob job(app_);
    job.setAutoDelete(false);
    job.setKey(name_);
    QByteArray data;
    QDataStream ds(&data,QIODevice::ReadWrite);
    ds << pairs_;

    job.setBinaryData(data);
    QEventLoop loop;
    job.connect( &job, SIGNAL(finished(QKeychain::Job*)), &loop, SLOT(quit()) );
    job.start();
    loop.exec();
    if(job.error())
    {
        qWarning() << "keychain could not be persisted "<< name_ << ":" << qPrintable(job.errorString());
    }
}

void o0keyChainStore::fetchFromKeychain() {
    ReadPasswordJob job(app_);
    job.setKey(name_);
    QEventLoop loop;
    job.connect( &job, SIGNAL(finished(QKeychain::Job*)), &loop, SLOT(quit()) );
    job.start();
    loop.exec();

    QByteArray data;
    data.append(job.binaryData());
    QDataStream ds(&data,QIODevice::ReadOnly);
    ds >> pairs_;

    if(job.error())
    {
        qWarning() << "keychain could not be fetched"<< name_ << ":" << qPrintable(job.errorString());
    }
}

void o0keyChainStore::clearFromKeychain() {
    DeletePasswordJob job(app_);
    job.setKey(name_);
    QEventLoop loop;
    job.connect( &job, SIGNAL(finished(QKeychain::Job*)), &loop, SLOT(quit()) );
    job.start();
    loop.exec();
    if ( job.error() ) {
        qWarning() << "Deleting keychain failed: " << qPrintable(job.errorString());
    }
}
