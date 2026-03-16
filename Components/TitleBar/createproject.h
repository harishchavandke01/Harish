#ifndef CREATEPROJECT_H
#define CREATEPROJECT_H

#include <QWidget>
#include <QPushButton>
#include <QString>
class CreateProject : public QWidget
{
    Q_OBJECT
public:
    explicit CreateProject(QWidget *parent = nullptr);


private:
    QPushButton * createProject;

signals:
    void projectFolderSelected(const QString &projectFolder);
};

#endif // CREATEPROJECT_H
