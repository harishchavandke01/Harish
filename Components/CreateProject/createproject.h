#ifndef CREATEPROJECT_H
#define CREATEPROJECT_H

#include <QWidget>

class QLabel;
class QPushButton;
class QLineEdit;

class CreateProject : public QWidget
{
    Q_OBJECT
public:
    explicit CreateProject(QWidget *parent = nullptr);

signals:
    void projectFolderSelected(const QString &folderPath);

private slots:
    void onCreateProjectClicked();

private:
    QLabel *titleLabel;
    QLabel *descriptionLabel;
    QLineEdit *folderLineEdit;
    QPushButton *createButton;
};

#endif // CREATEPROJECT_H
