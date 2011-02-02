#include "fancylistviewitem.h"

QFont FancyListViewItem::font(uint column) const
{
	if (column < fonts.size())
		return fonts[column];
	return listView()->font();
}

void FancyListViewItem::setFont(uint column, const QFont &font)
{
	if (column >= fonts.size())
		fonts.resize(column + 1, listView()->font());
	fonts[column] = font;
}

QColor FancyListViewItem::background(uint column) const
{
	if (column < backgrounds.size())
		return backgrounds[column];
	return listView()->colorGroup().base();
}

void FancyListViewItem::setBackground(uint column, const QColor &color)
{
	if (column >= backgrounds.size())
		backgrounds.resize(column + 1, listView()->colorGroup().base());
	backgrounds[column] = color;
}

void FancyListViewItem::paintCell(
								  QPainter *painter, const QColorGroup &cg,
								  int column, int width, int align)
{
	painter->save();
	if (column >= 0 && column < (int)fonts.size())
		painter->setFont(fonts[column]);
	QColorGroup grp(cg);
	if (column >= 0 && column < (int)backgrounds.size())
		grp.setColor(QColorGroup::Base, backgrounds[column]);
	QListViewItem::paintCell(painter, grp, column, width, align);
	painter->restore();
}

int FancyListViewItem::width(const QFontMetrics &fm, const QListView *lv, int column) const
{
	int width;
	if (column >= 0 && column < (int)fonts.size()) {
		QFontMetrics fm2(fonts[column]);
		width = QListViewItem::width(fm2, lv, column);
	}
	else
		width = QListViewItem::width(fm, lv, column);
	return width;
}

