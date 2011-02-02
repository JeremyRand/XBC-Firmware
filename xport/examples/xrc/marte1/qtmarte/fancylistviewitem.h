#include "qlistview.h"
#include "qvaluevector.h"
#include "qpainter.h"
#include "qfontmetrics.h"
#include "qstring.h"
#include "qcolor.h"
#include "qfont.h"

class FancyListViewItem : public QListViewItem
{
public:
	FancyListViewItem(QListView *parent, const QString &label1, const QString &label2 = NULL)
		: QListViewItem(parent, label1, label2)
	{}

	FancyListViewItem(QListViewItem *parent, const QString &label1, const QString &label2 = NULL)
		: QListViewItem(parent, label1, label2)
	{}

	void paintCell(QPainter *painter, const QColorGroup &cg,
		int column, int width, int align);
	int width(const QFontMetrics &fm, const QListView *lv, int column) const;

	QFont font(uint column) const;
	void setFont(uint column, const QFont &font);
	QColor background(uint column) const;
	void setBackground(uint column, const QColor &color);

private:
	QValueVector<QFont> fonts;
	QValueVector<QColor> backgrounds;
};
