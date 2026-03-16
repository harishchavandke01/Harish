#ifndef CUSTOMCHECKBOX_H
#define CUSTOMCHECKBOX_H

#include <QWidget>

class CustomCheckBox : public QWidget
{
    Q_OBJECT
public:
    explicit CustomCheckBox(QString s, bool val, bool en = true, QWidget *parent = nullptr);

    QSize sizeHint() const override;
    void setVal(bool val);
    bool isChecked() const;
    void setText(const QString& text);
    QString text() const;
    void updateSize();

signals:
    void toggled(bool checked);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    QString m_text;
    bool m_checked;
    const int checkboxSize = 25;
    const int spacing = 8;
    const int padding = 5;
};

#endif // CUSTOMCHECKBOX_H
