#include <qapplication.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qthread.h>
#include <qpushbutton.h>

#include "TcpController.h"

int main(int argc, char* argv[])
{
	QApplication a(argc, argv);
	//开启Tcp服务
	QThread* tcpThread = new QThread();
	TcpController::getInstance()->moveToThread(tcpThread);
	tcpThread->start();

	//写个界面做测试
	QPushButton* b = new QPushButton();
	b->connect(b, SIGNAL(clicked()), TcpController::getInstance(), SLOT(startServer()));
	QHBoxLayout* l = new QHBoxLayout();
	l->addWidget(b);
	QWidget* w = new QWidget();
	w->setLayout(l);
	w->show();
	return a.exec();
}
