#ifndef QTXHTTPSERVER_ABSTRACTHTTPREQUEST_H
#define QTXHTTPSERVER_ABSTRACTHTTPREQUEST_H

#include <QtNetwork>
#include <QtCore>


/*! The AbstractHttpRequest class provides the base functionality common to all HTTP requests.
 */
class AbstractHttpRequest : public QIODevice
{
	Q_OBJECT
	
public:
	static const QString kAppPathParam;
	static const QString kMountPathParam;
	
public:
	AbstractHttpRequest();
	virtual ~AbstractHttpRequest();
	
	virtual bool isValid() = 0;
	
	virtual QString version() const = 0;
	virtual QString uri() const = 0;
	virtual QString method() const = 0;
	virtual QString header(const QString & field) const;
	virtual QList<QString> headers(const QString & field) const;
	virtual QMultiHash<QString, QString> headers() const = 0;
	
	virtual QString parameter(const QString & key) const = 0;
	virtual void setParameter(const QString & key, const QString & value) = 0;
	
signals:
	/*! Emitted when the HTTP headers have been received.
	 */
	void received();
	
	/*! Emitted when the full HTTP request, including the body (if any), has been received.
	 */
	void complete();
};

#endif // QTXHTTPSERVER_ABSTRACTHTTPREQUEST_H
