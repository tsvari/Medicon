#pragma once

#include <QDateTimeEdit>
#include <QPalette>
#include <QColor>
#include <QString>
#include <QVariant>

#include <algorithm>

namespace GrpcDateTimeEditReadOnlyStyler {

inline void apply(QDateTimeEdit* dateTimeEdit, bool readOnly, double dimBlendWeight = 0.35)
{
    if (!dateTimeEdit) {
        return;
    }

    static constexpr const char* kOriginalPaletteProp = "_grpc_originalPalette";
    static constexpr const char* kOriginalStyleSheetProp = "_grpc_originalStyleSheet";

    const auto blend = [](const QColor& fg, const QColor& bg, double fgWeight) -> QColor {
        const double w = std::clamp(fgWeight, 0.0, 1.0);
        const double iw = 1.0 - w;
        return QColor::fromRgbF(
            fg.redF() * w + bg.redF() * iw,
            fg.greenF() * w + bg.greenF() * iw,
            fg.blueF() * w + bg.blueF() * iw,
            1.0);
    };

    if (readOnly) {
        if (!dateTimeEdit->property(kOriginalPaletteProp).isValid()) {
            dateTimeEdit->setProperty(kOriginalPaletteProp, QVariant::fromValue(dateTimeEdit->palette()));
        }
        if (!dateTimeEdit->property(kOriginalStyleSheetProp).isValid()) {
            dateTimeEdit->setProperty(kOriginalStyleSheetProp, dateTimeEdit->styleSheet());
        }

        // QDateEdit/QTimeEdit/QDateTimeEdit: setReadOnly() alone does not
        // make the control look read-only and can still allow changes via
        // stepping/wheel/calendar popup.
        dateTimeEdit->setReadOnly(true);
        dateTimeEdit->setEnabled(false);

        // Some styles (especially dark themes) keep disabled text too visible.
        // Force a much dimmer color by blending active text into the base color,
        // but ONLY for the disabled state (so normal/read-write remains readable).
        QPalette pal = dateTimeEdit->palette();
        const QColor activeText = pal.color(QPalette::Active, QPalette::Text);
        const QColor activeBase = pal.color(QPalette::Active, QPalette::Base);
        const QColor dimText = blend(activeText, activeBase, dimBlendWeight);
        const QColor disabledBase = pal.color(QPalette::Disabled, QPalette::Base);

        pal.setColor(QPalette::Disabled, QPalette::Text, dimText);
        pal.setColor(QPalette::Disabled, QPalette::WindowText, dimText);
        pal.setColor(QPalette::Disabled, QPalette::Base, disabledBase);

        dateTimeEdit->setPalette(pal);

        // Force the same color through style sheets as a last resort.
        // QAbstractSpinBox covers QDateEdit/QTimeEdit/QDateTimeEdit.
        const QString css = QString(
            "QAbstractSpinBox:disabled, QAbstractSpinBox:disabled QLineEdit { color: rgb(%1,%2,%3); }")
                                .arg(dimText.red())
                                .arg(dimText.green())
                                .arg(dimText.blue());

        const QString original = dateTimeEdit->property(kOriginalStyleSheetProp).toString();
        dateTimeEdit->setStyleSheet(original.isEmpty() ? css : (original + "\n" + css));
    } else {
        dateTimeEdit->setEnabled(true);
        dateTimeEdit->setReadOnly(false);

        const QVariant storedPalette = dateTimeEdit->property(kOriginalPaletteProp);
        if (storedPalette.isValid() && storedPalette.canConvert<QPalette>()) {
            dateTimeEdit->setPalette(storedPalette.value<QPalette>());
        }
        dateTimeEdit->setProperty(kOriginalPaletteProp, QVariant());

        const QVariant storedStyle = dateTimeEdit->property(kOriginalStyleSheetProp);
        if (storedStyle.isValid()) {
            dateTimeEdit->setStyleSheet(storedStyle.toString());
        }
        dateTimeEdit->setProperty(kOriginalStyleSheetProp, QVariant());
    }
}

} // namespace GrpcDateTimeEditReadOnlyStyler
