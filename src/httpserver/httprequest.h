#ifndef QTXHTTPSERVER_HTTPREQUEST_H
#define QTXHTTPSERVER_HTTPREQUEST_H

#include "abstracthttprequest.h"
#include <QtNetwork>
#include <QtCore>


/*! The HttpRequest class contains the data and headers for a HTTP request.
 */
class HttpRequest : public AbstractHttpRequest
{
	Q_OBJECT
	
public:
	static const QString kServerSoftwareParam;
	static const QString kServerAddressParam;
	static const QString kServerPortParam;
	static const QString kRemoteAddressParam;
	static const QString kProtocolParam;
	static const QString kAppPathParam;
	static const QString kMountPathParam;
	
	enum ParseError {
		NoError,
		BadRequestError,
		RequestEntityTooLargeError,
		RequestUriTooLongError,
		HttpVersionNotSupportedError,
		UnknownError
	};
	
public:
	/*! Constructs a HttpRequest object.
	 *
	 *  Ownership of \a ioDevice is not assumed and remains with the caller.
	 */
	HttpRequest(QIODevice* ioDevice);
	~HttpRequest();
	
	bool isValid();
	
	QString version() const;
	QString uri() const;
	QString method() const;
	QMultiHash<QString, QString> headers() const;
	
	QString parameter(const QString & key) const;
	void setParameter(const QString & key, const QString & value);
	
	qint64 bytesAvailable() const;
	
	void setStartLineBufferLimit(quint16 limit);
	void setHeadersBufferLimit(quint16 limit);
	
signals:
	/*! Emitted when an error is detected parsing the HTTP request.
	 */
	void error(ParseError code);
	
protected:
	virtual qint64 readData(char* data, qint64 maxSize);
	virtual qint64 writeData(const char* data, qint64 maxSize);
	
private:
	void setVersion(const QString & version);
	void setUri(const QString & uri);
	void setMethod(const QString & method);
	void addHeader(const QString & field, const QString & value);
	
	qint64 expectedContentLength();
	
	void parse();
	void parseHeader(const QByteArray & buffer);

private slots:
	void onReadyRead();
	
private:
	static const quint16 kStartLineBufferLimit;
	static const quint16 kHeadersBufferLimit;
	
	enum SectionType {
		NoSection,
		Invalid,
		StartLine,
		Headers,
		Body
	};
	
private:
	QPointer<QIODevice> _ioDevice;
	QByteArray _buffer;
	qint64 _bytesRead;
	bool _complete;
	
	QString _version;
	QString _uri;
	QString _method;
	QMultiHash<QString, QString> _headers;
	
	QHash<QString, QString> _parameters;
	
	SectionType _section;
	ParseError _parseError;
	
	quint16 _startLineBufferLimit;
	quint16 _headersBufferLimit;
};

#endif // QTXHTTPSERVER_HTTPREQUEST_H
