#include "createproject.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QFileDialog>

CreateProject::CreateProject(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("createProjectBox");

    titleLabel = new QLabel(tr("Create New Project"), this);
    titleLabel->setObjectName("createProjectTitle");

    descriptionLabel = new QLabel(
        tr("Select a folder where the project files will be stored."), this);
    descriptionLabel->setObjectName("createProjectDescription");
    descriptionLabel->setWordWrap(true);

    folderLineEdit = new QLineEdit(this);
    folderLineEdit->setObjectName("createProjectPath");
    folderLineEdit->setPlaceholderText(tr("No folder selected"));
    folderLineEdit->setReadOnly(true);

    createButton = new QPushButton(tr("Create Project"), this);
    createButton->setObjectName("createProjectButton");
    createButton->setCursor(Qt::PointingHandCursor);

    QHBoxLayout *rowLayout = new QHBoxLayout;
    rowLayout->addWidget(folderLineEdit, 1);
    rowLayout->addWidget(createButton, 0);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(8);

    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(descriptionLabel);
    mainLayout->addLayout(rowLayout);

    setLayout(mainLayout);

    connect(createButton, &QPushButton::clicked, this, &CreateProject::onCreateProjectClicked);
}

void CreateProject::onCreateProjectClicked()
{
    QString dir = QFileDialog::getExistingDirectory(
        this,
        tr("Select Project Folder"),
        QString(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );

    if (dir.isEmpty())
        return;

    folderLineEdit->setText(dir);
    emit projectFolderSelected(dir);
}
