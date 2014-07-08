#pragma once

#include "FractoriumPch.h"

/// <summary>
/// EmberTreeWidgetItem
/// </summary>

/// <summary>
/// A thin derivation of QTreeWidgetItem for a tree of embers in an open file.
/// The tree is intended to contain one open ember file at a time.
/// This is a non-templated base for casting purposes.
/// </summary>
class EmberTreeWidgetItemBase : public QTreeWidgetItem
{
public:
	/// <summary>
	/// Constructor that takes a pointer to a QTreeWidget as a parent widget.
	/// This is meant to be a root level item.
	/// </summary>
	/// <param name="parent">The parent widget of this item</param>
	explicit EmberTreeWidgetItemBase(QTreeWidget* parent = 0)
		: QTreeWidgetItem(parent)
	{
	}

	/// <summary>
	/// Constructor that takes a pointer to a QTreeWidgetItem as a parent widget.
	/// This is meant to be the child of a root level item.
	/// </summary>
	/// <param name="parent">The parent widget of this item</param>
	explicit EmberTreeWidgetItemBase(QTreeWidgetItem* parent = 0)
		: QTreeWidgetItem(parent)
	{
	}
	
	/// <summary>
	/// Set the preview image for the tree widget item.
	/// </summary>
	/// <param name="v">The vector containing the RGB pixels [0..255] which will make up the preview image</param>
	/// <param name="width">The width of the image in pixels</param>
	/// <param name="height">The height of the image in pixels</param>
	void SetImage(vector<unsigned char>& v, unsigned int width, unsigned int height)
	{
		int size = 64;

		m_Image = QImage(width, height, QImage::Format_RGBA8888);
		memcpy(m_Image.scanLine(0), v.data(), v.size() * sizeof(v[0]));//Memcpy the data in.
		m_Pixmap = QPixmap::fromImage(m_Image).scaled(QSize(size, size), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);//Create a QPixmap out of the QImage, scaled to size.
		setData(0, Qt::DecorationRole, m_Pixmap);
	}

protected:
	QImage m_Image;
	QPixmap m_Pixmap;
};

/// <summary>
/// A thin derivation of QTreeWidgetItem for a tree of embers in an open file.
/// The tree is intended to contain one open ember file at a time.
/// </summary>
template <typename T>
class EmberTreeWidgetItem : public EmberTreeWidgetItemBase
{
public:
	/// <summary>
	/// Constructor that takes a pointer to an ember and a QTreeWidget as a parent widget.
	/// This is meant to be a root level item.
	/// </summary>
	/// <param name="ember">A pointer to the ember this item will represent</param>
	/// <param name="parent">The parent widget of this item</param>
	explicit EmberTreeWidgetItem(Ember<T>* ember, QTreeWidget* parent = 0)
		: EmberTreeWidgetItemBase(parent)
	{
		m_Ember = ember;
	}

	/// <summary>
	/// Constructor that takes a pointer to an ember and a QTreeWidgetItem as a parent widget.
	/// This is meant to be the child of a root level item.
	/// </summary>
	/// <param name="ember">A pointer to the ember this item will represent</param>
	/// <param name="parent">The parent widget of this item</param>
	explicit EmberTreeWidgetItem(Ember<T>* ember, QTreeWidgetItem* parent = 0)
		: EmberTreeWidgetItemBase(parent)
	{
		m_Ember = ember;
	}

	/// <summary>
	/// Copy the text of the tree item to the name of the ember.
	/// </summary>
	void UpdateEmberName() { m_Ember->m_Name = text(0).toStdString(); }

	/// <summary>
	/// Set the text of the tree item.
	/// </summary>
	void UpdateEditText() { setText(0, QString::fromStdString(m_Ember->m_Name)); }

	/// <summary>
	/// Get a pointer to the ember held by the tree item.
	/// </summary>
	Ember<T>* GetEmber() const { return m_Ember; }

	/// <summary>
	/// Perform a deep copy from the passed in ember to the dereferenced
	/// ember pointer of the tree item.
	/// </summary>
	/// <param name="ember">The ember to copy</param>
	void SetEmber(Ember<T>& ember) { *m_Ember = ember; }

	/// <summary>
	/// Set the ember pointer member to point to the passed in ember pointer.
	/// </summary>
	/// <param name="ember">The ember to point to</param>
	void SetEmberPointer(Ember<T>* ember) { m_Ember = ember; }

private:
	Ember<T>* m_Ember;
};
