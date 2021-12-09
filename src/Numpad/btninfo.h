#ifndef BTNINFO_H
#define BTNINFO_H

#include <QString>
#include <QList>

enum BtnShape { Sqr, BigHor, BigVer };

struct BtnStaticInfo
{
    QString view;
    bool useAltCode;
    QList<int> codes;
    BtnStaticInfo(QString _view, bool _useAltCode, QList<int> _codes)
      { view = _view; useAltCode = _useAltCode; codes = _codes; }
};

struct BtnDynamicInfo
{
    QList<int> ids;
    BtnShape shape;
    int row;
    int column;
    BtnDynamicInfo(QList<int> _ids, BtnShape _shape, int _row, int _column)
      { ids = _ids; shape = _shape; row = _row; column = _column; }
};

#endif // BTNINFO_H
