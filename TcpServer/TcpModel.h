#pragma once

#include <QObject>
#include <QThread>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonDocument>
#include <QJsonArray>
#include <QCrypToGraphicHash>
#include <QHostAddress>
#include <QFile>
#include <QFileInfo>
#include <QPixmap>
#include <QBitmap>
#include <QApplication>
#include <QDateTime>

#include "QByteArrayHelper.h"
#include "TcpData.h"

#define TCPMODEL_MD5_CHECK 1
#define TCPMODEL_DATA_HEAD_SIZE 4
#define TCPMODEL_BUF_SIZE 10*1024*1024

/// <summary>
/// Tcp业务，处理接收的数据，和将要发送的数据
/// </summary>
class TcpModel : public QObject
{
	Q_OBJECT
public:
	TcpModel(qintptr socketDescriptor);
	~TcpModel();
public slots:
	//To Client
	void sendMsgToClient(QString msg, int state = 0);		//发送消息
	void sendFileToClient(QString filePath);				//发送文件(先读入sendList，再发送头)
private:
	qintptr socketDescriptor;								//客户端套接字指针
	QTcpSocket* socket;										//客户端套接字

	void createConnections();								//创建连接

	//send
	QList<TcpData*> sendList;								//发送消息的列表
	void sendHeadToClient(QString type, QString fileName, quint64 fileSize, int state, QString md5 = "");//发送Json头
	void sendDataToClient(QString type, QString fileName, quint64 fileSize);	//发送数据

	//recevice
	QByteArray partialBlockBuf = {};
	QList<TcpData*> receList;								//接收消息的列表
	void receAnalysisFromClient(QByteArray buf);			//对接收到的数据进行解析
	void receJsonFromClient(QJsonObject obj);				//对接到的json数据进行二次解析
	void receHeadFromClient(QString type, QString fileName, quint64 fileSize, QString md5);	//解析Json得到的是头
	void receMsgFromClient(QString msg, QString name, QString md5, int state);//解析Json得到的是消息
	void receDataFromClient(QByteArray buf);				//接到数据(字节数组)

	//analysis
	void analysisData(QString fileName, QByteArray buf, QString type = "message");//初步解析数据
	void analysisMSG(QString fileName, QString buf);		//解析信息
	void analysisBMP(QString fileName, QByteArray buf);		//解析bmp数据
	void analysisPNG(QString fileName, QByteArray buf);		//解析png数据
	void analysisCSV(QString fileName, QByteArray buf);		//解析csv数据
	void analysisJson(QString fileName, QByteArray buf);	//解析json数据

private slots:
	//From socket
	void readyRead();										//接收到消息预处理(初步解析，此处已经规定好格式)
	//From Data
	void clearMember(TcpData* data);						//清除成员
signals:
	//To Thread
	void disconnected();
	void sendMsgToThread(QString msg);									//发送消息
};