// scheduledialog.cpp
#include "scheduledialog.h"
#include <QVBoxLayout>
#include <QListWidget>
#include <QTimeEdit>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QListWidgetItem>
#include <QDebug>
#include <QLabel>

ScheduleDialog::ScheduleDialog(QWidget* parent, const QList<QTime>& schedule)
    : QDialog(parent)
    , m_schedule(schedule)
{
    qDebug() << "Initializing ScheduleDialog with schedule:" << schedule;

    QVBoxLayout* layout = new QVBoxLayout(this);

    // Time input
    m_timeEdit = new QTimeEdit(this);
    m_timeEdit->setDisplayFormat("hh:mm:ss");
    layout->addWidget(m_timeEdit);

    // List of scheduled times
    m_listWidget = new QListWidget(this);
    layout->addWidget(m_listWidget);

    // Populate
    for (const QTime& time : m_schedule) {
        m_listWidget->addItem(time.toString("hh:mm:ss"));
    }

    // Add/Remove buttons
    QPushButton* addButton = new QPushButton("Add", this);
    connect(addButton, &QPushButton::clicked, this, &ScheduleDialog::addTimer);
    layout->addWidget(addButton);

    QPushButton* removeButton = new QPushButton("Remove", this);
    connect(removeButton, &QPushButton::clicked, this, &ScheduleDialog::removeTimer);
    layout->addWidget(removeButton);

    // **Total time label**
    m_totalLabel = new QLabel(this);
    layout->addWidget(m_totalLabel);

    // OK/Cancel
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this
    );
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttonBox);

    setLayout(layout);
    setWindowTitle("Edit Schedule");

    // Update total label initially
    updateTotalLabel();
}

void ScheduleDialog::addTimer()
{
    QTime newTime = m_timeEdit->time();
    qDebug() << "Adding new time to schedule:" << newTime.toString("hh:mm:ss");
    m_schedule.append(newTime);
    m_listWidget->addItem(newTime.toString("hh:mm:ss"));

    updateTotalLabel();
}

void ScheduleDialog::removeTimer()
{
    int row = m_listWidget->currentRow();
    if (row >= 0 && row < m_listWidget->count()) {
        qDebug() << "Removing time at row:" << row << "Time:" << m_schedule[row].toString("hh:mm:ss");
        m_schedule.removeAt(row);
        delete m_listWidget->takeItem(row);
        updateTotalLabel();
    }
    else {
        qDebug() << "No valid row selected for removal.";
    }
}

QList<QTime> ScheduleDialog::getSchedule() const
{
    qDebug() << "Returning schedule:" << m_schedule;
    return m_schedule;
}

void ScheduleDialog::updateTotalLabel()
{
    // Sum all QTimes in m_schedule
    qint64 totalSeconds = 0;
    for (const QTime& t : m_schedule) {
        totalSeconds += QTime(0, 0).secsTo(t);
    }
    // Convert totalSeconds back to a QTime
    QTime total(0, 0);
    total = total.addSecs(totalSeconds);

    QString totalTimeString = total.toString("hh:mm:ss");
    qDebug() << "Total time calculated:" << totalTimeString;
    m_totalLabel->setText("Total time: " + totalTimeString);
}