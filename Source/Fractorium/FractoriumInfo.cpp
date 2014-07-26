#include "FractoriumPch.h"
#include "Fractorium.h"

/// <summary>
/// Update the histogram bounds display labels.
/// This shows the user the actual bounds of what's
/// being rendered. Mostly of engineering interest.
/// </summary>
void Fractorium::UpdateHistogramBounds()
{
	if (RendererBase* r = m_Controller->Renderer())
	{
		sprintf_s(m_ULString, 32, "UL: %3.3f, %3.3f",  r->LowerLeftX(),  r->UpperRightY());//These bounds include gutter padding.
		sprintf_s(m_URString, 32, "UR: %3.3f, %3.3f", -r->LowerLeftX(),  r->UpperRightY());
		sprintf_s(m_LRString, 32, "LR: %3.3f, %3.3f", -r->LowerLeftX(),  r->LowerLeftY());
		sprintf_s(m_LLString, 32, "LL: %3.3f, %3.3f",  r->LowerLeftX(),  r->LowerLeftY());
		sprintf_s(m_WString,  16,  "W: %4d"			,  r->SuperRasW());
		sprintf_s(m_HString,  16,  "H: %4d"			,  r->SuperRasH());

		ui.InfoBoundsLabelUL->setText(QString(m_ULString));
		ui.InfoBoundsLabelUR->setText(QString(m_URString));
		ui.InfoBoundsLabelLR->setText(QString(m_LRString));
		ui.InfoBoundsLabelLL->setText(QString(m_LLString));
		ui.InfoBoundsLabelW->setText(QString(m_WString));
		ui.InfoBoundsLabelH->setText(QString(m_HString));

		ui.InfoBoundsTable->item(0, 1)->setText(QString::number(r->GutterWidth()));

		if (r->GetDensityFilter())
		{
			unsigned int deWidth = (r->GetDensityFilter()->FilterWidth() * 2) + 1;

			sprintf_s(m_DEString, 16, "%d x %d", deWidth, deWidth);
			ui.InfoBoundsTable->item(1, 1)->setText(QString(m_DEString));
		}
		else
			ui.InfoBoundsTable->item(1, 1)->setText("N/A");
	}
}

/// <summary>
/// Fill the passed in QTextEdit with the vector of strings.
/// Optionally clear first.
/// Serves as a convenience function because the error reports coming
/// from Ember and EmberCL use vector<string>.
/// Use invokeMethod() in case this is called from a thread.
/// </summary>
/// <param name="errors">The vector of error strings</param>
/// <param name="textEdit">The QTextEdit to fill</param>
/// <param name="clear">Clear if true, else don't.</param>
void Fractorium::ErrorReportToQTextEdit(vector<string>& errors, QTextEdit* textEdit, bool clear)
{
	if (clear)
		QMetaObject::invokeMethod(textEdit, "clear", Qt::QueuedConnection);

	for (size_t i = 0; i < errors.size(); i++)
		QMetaObject::invokeMethod(textEdit, "append", Qt::QueuedConnection, Q_ARG(QString, QString::fromStdString(errors[i]) + "\n"));
}