#pragma once

#include "FractoriumCommon.h"

/// <summary>
/// EmberFile class.
/// </summary>

/// <summary>
/// Class for representing an ember Xml file in memory.
/// It contains a filename and a vector of embers.
/// It also provides static helper functions for creating
/// default names for the file and the embers in it.
/// </summary>
template <typename T>
class EmberFile
{
public:
	/// <summary>
	/// Empty constructor that does nothing.
	/// </summary>
	EmberFile()
	{
	}

	/// <summary>
	/// Default copy constructor.
	/// </summary>
	/// <param name="emberFile">The EmberFile object to copy</param>
	EmberFile(const EmberFile<T>& emberFile)
	{
		EmberFile<T>::operator=<T>(emberFile);
	}

	/// <summary>
	/// Copy constructor to copy an EmberFile object of type U.
	/// </summary>
	/// <param name="emberFile">The EmberFile object to copy</param>
	template <typename U>
	EmberFile(const EmberFile<U>& emberFile)
	{
		EmberFile<T>::operator=<U>(emberFile);
	}

	/// <summary>
	/// Default assignment operator.
	/// </summary>
	/// <param name="emberFile">The EmberFile object to copy</param>
	EmberFile<T>& operator = (const EmberFile<T>& emberFile)
	{
		if (this != &emberFile)
			EmberFile<T>::operator=<T>(emberFile);

		return *this;
	}

	/// <summary>
	/// Assignment operator to assign a EmberFile object of type U.
	/// </summary>
	/// <param name="emberFile">The EmberFile object to copy.</param>
	/// <returns>Reference to updated self</returns>
	template <typename U>
	EmberFile<T>& operator = (const EmberFile<U>& emberFile)
	{
		m_Filename = emberFile.m_Filename;
		CopyVec(m_Embers, emberFile.m_Embers);
		return *this;
	}

	/// <summary>
	/// Clear the file name and the vector of embers.
	/// </summary>
	void Clear()
	{
		m_Filename.clear();
		m_Embers.clear();
	}

	/// <summary>
	/// Thin wrapper to get the size of the vector of embers.
	/// </summary>
	size_t Size()
	{
		return m_Embers.size();
	}

	/// <summary>
	/// Delete the ember at the given index.
	/// Will not delete anything if the size is already 1.
	/// </summary>
	/// <param name="index">The index of the ember to delete</param>
	/// <returns>True if successfully deleted, else false.</returns>
	bool Delete(size_t index)
	{
		if (Size() > 1 && index < Size())
		{
			m_Embers.erase(m_Embers.begin() + index);
			return true;
		}
		else
			return false;
	}

	/// <summary>
	/// Ensure all ember names are unique.
	/// </summary>
	void MakeNamesUnique()
	{
		int x = 0;

		for (size_t i = 0; i < m_Embers.size(); i++)
		{
			for (size_t j = 0; j < m_Embers.size(); j++)
			{
				if (i != j && m_Embers[i].m_Name == m_Embers[j].m_Name)
				{
					m_Embers[j].m_Name = m_Embers[j].m_Name + "_" + ToString(++x).toStdString();
					j = 0;
				}
			}
		}
	}

	/// <summary>
	/// Return the default filename based on the current date/time.
	/// </summary>
	/// <returns>The default filename</returns>
	static QString DefaultFilename()
	{
		return "Flame_" + QDateTime(QDateTime::currentDateTime()).toString("yyyy-MM-dd-hhmmss");
	}

	/// <summary>
	/// Ensures a given input filename is unique by appending a count to the end.
	/// </summary>
	/// <param name="filename">The filename to ensure is unique</param>
	/// <returns>The passed in name if it was unique, else a uniquely made name.</returns>
	static QString UniqueFilename(const QString& filename)
	{
		if (!QFile::exists(filename))
			return filename;

		int counter = 2;
		QString newPath;
		QFileInfo original(filename);
		QString path = original.absolutePath() + '/';
		QString base = original.completeBaseName();
		QString extension = original.suffix();
		
		do
		{
			newPath = path + base + "_" + ToString(counter++) + "." + extension;
		}
		while (QFile::exists(newPath));
		
		return newPath;
	}

	/// <summary>
	/// Return the default ember name based on the current date/time and
	/// the ember's index in the file.
	/// </summary>
	/// <param name="i">The index in the file of the ember</param>
	/// <returns>The default ember name</returns>
	static QString DefaultEmberName(uint i)
	{
		return DefaultFilename() + "-" + ToString(i);
	}

	QString m_Filename;
	vector<Ember<T>> m_Embers;
};
