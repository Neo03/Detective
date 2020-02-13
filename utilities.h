#ifndef UTILITIES_H
#define UTILITIES_H

#include <QString>

class Utilities{
    public:

    static QString getDataPath();
    static QString newPhotoName();
    static QString getPhotoPath(QString name, QString postfix);

};

#endif // UTILITIES_H

