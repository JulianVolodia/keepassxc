/*
 *  Copyright (C) 2017 KeePassXC Team
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 or (at your option)
 *  version 3 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cstdlib>
#include <stdio.h>

#include "Show.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFile>
#include <QStringList>
#include <QTextStream>

#include "core/Database.h"
#include "core/Entry.h"
#include "core/Group.h"
#include "format/KeePass2Reader.h"
#include "keys/CompositeKey.h"

int Show::execute(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QTextStream out(stdout);

    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::translate("main",
                                                                 "Show a password."));
    parser.addPositionalArgument("database", QCoreApplication::translate("main", "Path of the database."));
    parser.addPositionalArgument("uuid", QCoreApplication::translate("main", "Uuid of the entry to show"));
    parser.process(app);

    const QStringList args = parser.positionalArguments();
    if (args.size() != 2) {
        parser.showHelp();
        return EXIT_FAILURE;
    }

    out << "Insert the database password\n> ";
    out.flush();

    static QTextStream inputTextStream(stdin, QIODevice::ReadOnly);
    QString line = inputTextStream.readLine();
    CompositeKey key = CompositeKey::readFromLine(line);

    QString databaseFilename = args.at(0);
    QFile dbFile(databaseFilename);
    if (!dbFile.exists()) {
        qCritical("File %s does not exist.", qPrintable(databaseFilename));
        return EXIT_FAILURE;
    }
    if (!dbFile.open(QIODevice::ReadOnly)) {
        qCritical("Unable to open file %s.", qPrintable(databaseFilename));
        return EXIT_FAILURE;
    }

    KeePass2Reader reader;
    Database* db = reader.readDatabase(&dbFile, key);

    if (reader.hasError()) {
        qCritical("Error while parsing the database:\n%s\n", qPrintable(reader.errorString()));
        return EXIT_FAILURE;
    }

    Uuid uuid = Uuid::fromHex(args.at(1));
    Entry* entry = db->resolveEntry(uuid);
    out << entry->password() << "\n";
    return EXIT_SUCCESS;
}
