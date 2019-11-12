/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Author: Milian Wolff, KDAB (milian.wolff@kdab.com)
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#include <QCoreApplication>
#include <QStringList>
#include <QTextStream>
#include <QDir>
#include <QTcpSocket>
#include <QXmlStreamReader>
#include <QProcessEnvironment>

#include "outputgenerator.h"

using namespace Valgrind::Fake;

using namespace Qt;

QTextStream qerr(stderr);
QTextStream qout(stdout);

void usage(QTextStream& stream)
{
    stream << "valgrind-fake OPTIONS" << endl;
    stream << endl;
    stream << " REQUIRED OPTIONS:" << endl;
    stream << "  --xml-socket=ipaddr:port \tXML output to socket ipaddr:port" << endl;
    stream << "  -i, --xml-input FILE     \tpath to a XML file as generated by valgrind" << endl;
    stream << endl;
    stream << " OPTIONAL OPTIONS:" << endl;
    stream << "  -c, --crash              \tcrash randomly" << endl;
    stream << "  -g, --garbage            \toutput invalid XML somewhere" << endl;
    stream << "  -w, --wait SECONDS       \twait randomly for the given amount of seconds" << endl;
    stream << "  -h, --help               \tprint help" << endl;
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    const QStringList args = app.arguments();
    QString arg_port;
    QString arg_server;
    QString arg_xmlFile;
    bool arg_crash = false;
    bool arg_garbage = false;
    uint arg_wait = 0;

    const QProcessEnvironment sysEnv = QProcessEnvironment::systemEnvironment();
    arg_xmlFile = sysEnv.value(QLatin1String("QCIT_INPUT_FILE"));

    for (int i = 1; i < args.size(); ++i) {
        const QString& arg = args.at(i);
        if (arg.startsWith(QLatin1String("--xml-socket="))) {
            arg_server = arg.mid(13, arg.indexOf(QLatin1Char(':')) - 13);
            arg_port = arg.mid(13 + arg_server.length() + 1);
        } else if (args.size() > i + 1
                    && (args.at(i) == QLatin1String("-i")
                        || args.at(i) == QLatin1String("--xml-input"))) {
            arg_xmlFile = args.at(i+1);
            ++i;
        } else if (arg == QLatin1String("-c") || arg == QLatin1String("--crash")) {
            arg_crash = true;
        } else if (arg == QLatin1String("-g") || arg == QLatin1String("--garbage")) {
            arg_garbage = true;
        } else if (args.size() > i + 1 && (arg == QLatin1String("-w") || arg == QLatin1String("--wait"))) {
            bool ok;
            arg_wait = args.at(i+1).toUInt(&ok);
            if (!ok) {
                qerr << "ERROR: invalid wait time given" << args.at(i+1) << endl;
                usage(qerr);
                return 4;
            }
        } else if (args.at(i) == QLatin1String("--help") || args.at(i) == QLatin1String("-h")) {
            usage(qout);
            return 0;
        }
    }

    if (arg_xmlFile.isEmpty()) {
        qerr << "ERROR: no XML input file given" << endl;
        usage(qerr);
        return 1;
    }
    if (arg_server.isEmpty()) {
        qerr << "ERROR: no server given" << endl;
        usage(qerr);
        return 2;
    }
    if (arg_port.isEmpty()) {
        qerr << "ERROR: no port given" << endl;
        usage(qerr);
        return 3;
    }

    QFile xmlFile(arg_xmlFile);
    if (!xmlFile.exists() || !xmlFile.open(QIODevice::ReadOnly)) {
        qerr << "ERROR: invalid XML file" << endl;
        usage(qerr);
        return 10;
    }
    bool ok = false;
    quint16 port = arg_port.toUInt(&ok);
    if (!ok) {
        qerr << "ERROR: invalid port" << endl;
        usage(qerr);
        return 30;
    }

    QTcpSocket socket;
    socket.connectToHost(arg_server, port, QIODevice::WriteOnly);
    if (!socket.isOpen()) {
        qerr << "ERROR: could not open socket to server:" << arg_server << ":" << port << endl;
        usage(qerr);
        return 20;
    }
    if (!socket.waitForConnected()) {
        qerr << "ERROR: could not connect to socket: " << socket.errorString() << endl;
        return 21;
    }

    OutputGenerator generator(&socket, &xmlFile);
    QObject::connect(&generator, &OutputGenerator::finished, &app, &QCoreApplication::quit);
    generator.setCrashRandomly(arg_crash);
    generator.setOutputGarbage(arg_garbage);
    generator.setWait(arg_wait);

    return app.exec();
}
