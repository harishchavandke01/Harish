#ifndef ADJUSTOPTIONS_H
#define ADJUSTOPTIONS_H

#include <QDialog>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QGuiApplication>
#include <QScreen>
#include <QMouseEvent>

// ── Options data — shared between AdjustOptions dialog and LSSolver ────────
struct AdjustmentOptions {
    bool   useCovariance = true;   // use full 3×3 cov from batchls
    double aPrioriScalar = 1.0;    // multiplies all weights (1.0 = as-is)
    double defaultSigmaH = 0.010;  // m — fallback horizontal sigma
    double defaultSigmaV = 0.020;  // m — fallback vertical sigma
};

// NOTE: The old AdjustOptions dialog is no longer shown as a standalone
// button. Weighting options are now embedded in Tab 2 of AdjustNetworkDialog.
// This file is kept for the struct definition only.
// The AdjustOptions class below is intentionally left minimal.

#endif // ADJUSTOPTIONS_H
