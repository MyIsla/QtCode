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
/// Tcpҵ�񣬴�����յ����ݣ��ͽ�Ҫ���͵�����
/// </summary>
class TcpModel : public QObject
{
	Q_OBJECT
public:
	TcpModel(qintptr socketDescriptor);
	~TcpModel();
public slots:
	//To Client
	void sendMsgToClient(QString msg, int state = 0);		//������Ϣ
	void sendFileToClient(QString filePath);				//�����ļ�(�ȶ���sendList���ٷ���ͷ)
private:
	qintptr socketDescriptor;								//�ͻ����׽���ָ��
	QTcpSocket* socket;										//�ͻ����׽���

	void createConnections();								//��������

	//send
	QList<TcpData*> sendList;								//������Ϣ���б�
	void sendHeadToClient(QString type, QString fileName, quint64 fileSize, int state, QString md5 = "");//����Jsonͷ
	void sendDataToClient(QString type, QString fileName, quint64 fileSize);	//��������

	//recevice
	QByteArray partialBlockBuf = {};
	QList<TcpData*> receList;								//������Ϣ���б�
	void receAnalysisFromClient(QByteArray buf);			//�Խ��յ������ݽ��н���
	void receJsonFromClient(QJsonObject obj);				//�Խӵ���json���ݽ��ж��ν���
	void receHeadFromClient(QString type, QString fileName, quint64 fileSize, QString md5);	//����Json�õ�����ͷ
	void receMsgFromClient(QString msg, QString name, QString md5, int state);//����Json�õ�������Ϣ
	void receDataFromClient(QByteArray buf);				//�ӵ�����(�ֽ�����)

	//analysis
	void analysisData(QString fileName, QByteArray buf, QString type = "message");//������������
	void analysisMSG(QString fileName, QString buf);		//������Ϣ
	void analysisBMP(QString fileName, QByteArray buf);		//����bmp����
	void analysisPNG(QString fileName, QByteArray buf);		//����png����
	void analysisCSV(QString fileName, QByteArray buf);		//����csv����
	void analysisJson(QString fileName, QByteArray buf);	//����json����

private slots:
	//From socket
	void readyRead();										//���յ���ϢԤ����(�����������˴��Ѿ��涨�ø�ʽ)
	//From Data
	void clearMember(TcpData* data);						//�����Ա
signals:
	//To Thread
	void disconnected();
	void sendMsgToThread(QString msg);									//������Ϣ
};