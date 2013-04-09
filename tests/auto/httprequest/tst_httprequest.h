#ifndef TST_MOCKHTTPREQUEST_H
#define TST_MOCKHTTPREQUEST_H

#include "httprequest.h"
#include <QtTest/QtTest>

//QTX_USE_NAMESPACE


class TestHttpRequest : public QObject
{
	Q_OBJECT

private slots:
	void initTestCase();
	void init();
	void cleanup();

	void getMethod();
	void postMethod();

	void invalidProtocolError();
	void requestEntityTooLargeError();
	void requestUriTooLongError();
	void httpVersionNotSupportedError();

public slots:
	void onReceived();
	void onReadyRead();
	void onComplete();
	void onError(HttpRequest::ParseError code);

private:
	HttpRequest::ParseError _error;

	bool _emitsReceived;
	bool _emitsReadyRead;
	bool _emitsComplete;
	bool _emitsError;
};

#endif // TST_MOCKHTTPREQUEST_H
