// settingsdialog.h

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QMap>
#include <QLabel>  // Include QLabel header
#include "mainwindow.h"  // Include the new header file

class QLineEdit;

class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget* parent = nullptr, const QMap<MainWindow::Action, QString>& actionNameMap = {});

    void setKeyMap(const QMap<MainWindow::Action, int>& actionKeyMap);
    QMap<MainWindow::Action, int> getKeyMap() const;

private slots:
    void onOkClicked();
    void onCancelClicked();

private:
    struct ActionEdit {
        QLabel* label;
        QLineEdit* lineEdit;
        MainWindow::Action action;
    };

    QVector<ActionEdit> m_actionsEdits;
    QMap<MainWindow::Action, int> m_tempKeyMap;
    QMap<MainWindow::Action, QString> m_actionNameMap; // Add this line


    void buildUI();
};

#endif // SETTINGSDIALOG_H