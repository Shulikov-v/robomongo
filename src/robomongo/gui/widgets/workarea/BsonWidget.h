#pragma once

#include <QWidget>
#include <QtGui>
#include <mongo/client/dbclient.h>

#include "robomongo/core/Core.h"
#include "robomongo/core/domain/MongoDocument.h"

class QPlainTextEdit;

namespace Robomongo
{
    class BsonTreeWidget;

    /*
    ** Represents list of bson objects
    */
    class BsonWidget : public QWidget
    {
        Q_OBJECT

    public:
        BsonWidget(QWidget *parent = NULL);
        ~BsonWidget() {}
        void setDocuments(const QList<MongoDocumentPtr> &documents);

    private:
        BsonTreeWidget * _bsonTree;
    };


    /*
    ** In this thread we are running task to prepare JSON string from list of BSON objects
    */
    class JsonPrepareThread : public QThread
    {
        Q_OBJECT

    private:
        /*
        ** List of documents
        */
        QList<MongoDocumentPtr> _bsonObjects;

    public:
        /*
        ** Constructor
        */
        JsonPrepareThread(QList<MongoDocumentPtr> bsonObjects) : exit(false)
        {
            _bsonObjects = bsonObjects;
        }

        volatile bool exit;

    protected:

        /*
        ** Overload function
        */
        void run()
        {
            int position = 0;
            foreach(MongoDocumentPtr doc, _bsonObjects)
            {
                mongo::StringBuilder sb;
                if (position == 0)
                    sb << "/* 0 */\n";
                else
                    sb << "\n\n/* " << position << "*/\n";

                std::string stdJson = doc->bsonObj().jsonString(mongo::TenGen, 1);

                if (exit) {
                    emit done();
                    return;
                }

                sb << stdJson;
                QString json = QString::fromStdString(sb.str());

                if (exit) {
                    emit done();
                    return;
                }

                emit partReady(json);

                position++;
            }

            emit done();
        }

    signals:
        /**
         * @brief Signals when all parts prepared
         */
        void done();

        /**
         * @brief Signals when json part is ready
         */
        void partReady(const QString &part);
    };
}