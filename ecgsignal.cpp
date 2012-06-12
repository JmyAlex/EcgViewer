//#include "ecgsignal.h"
//#include <iostream>

//bool EcgSignal::readFile(const QString &fileName)
//{
//    QFile InFile(fileName);
//    if (!InFile.open(QIODevice::ReadOnly))
//    {
//        std::cerr << "Cannot open file for reading: " << qPrintable(InFile.errorString()) << std::endl;
//        return false;
//    }
//	_size = InFile.size() / sizeof(double);
//	double temp;
//	for (int i = 0; i < _size; i++)
//    {
//        InFile.read(reinterpret_cast<char*>(&temp), sizeof(double));
//		_data.push_back(temp);
//    }
//    InFile.close();
//    return true;
//}
