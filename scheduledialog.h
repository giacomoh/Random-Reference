#ifndef SCHEDULEDIALOG_H
#define SCHEDULEDIALOG_H

#include <QDialog>
#include <QList>
#include <QTime>

class QListWidget;
class QTimeEdit;
class QLabel;

class ScheduleDialog : public QDialog
{
    Q_OBJECT
public:
    // Pass the existing schedule into the constructor
    explicit ScheduleDialog(QWidget* parent, const QList<QTime>& schedule = QList<QTime>());

    // Method to retrieve the updated schedule
    QList<QTime> getSchedule() const;

private slots:
    void addTimer();
    void removeTimer();

private:
    void updateTotalLabel();  // new helper method

    QListWidget* m_listWidget;   // Displays the list of QTime items
    QTimeEdit* m_timeEdit;       // For inputting a new time
    QLabel* m_totalLabel;        // To display the total scheduled time
    QList<QTime> m_schedule;     // Internal copy of the schedule for editing
};

#endif // SCHEDULEDIALOG_H
